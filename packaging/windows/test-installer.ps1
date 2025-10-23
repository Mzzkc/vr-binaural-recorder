# VR Binaural Recorder Installer Testing Framework
# PowerShell script for validating installer on clean Windows systems

param(
    [Parameter(Mandatory=$true)]
    [string]$InstallerPath,

    [Parameter(Mandatory=$false)]
    [switch]$UninstallAfterTest = $true,

    [Parameter(Mandatory=$false)]
    [switch]$Verbose = $false,

    [Parameter(Mandatory=$false)]
    [string]$ReportPath = "installer-test-report.json"
)

# Test result structure
$TestResults = @{
    Timestamp = Get-Date -Format "yyyy-MM-dd HH:mm:ss"
    SystemInfo = @{}
    InstallerTests = @{}
    ApplicationTests = @{}
    UninstallTests = @{}
    OverallResult = $false
    Recommendations = @()
}

function Write-TestLog {
    param([string]$Message, [string]$Level = "INFO")
    $timestamp = Get-Date -Format "HH:mm:ss"
    $formattedMessage = "[$timestamp] [$Level] $Message"
    Write-Host $formattedMessage
    if ($Verbose) {
        Add-Content -Path "installer-test.log" -Value $formattedMessage
    }
}

function Test-SystemRequirements {
    Write-TestLog "Testing system requirements..."

    $TestResults.SystemInfo = @{
        OSVersion = [System.Environment]::OSVersion.VersionString
        Architecture = [System.Environment]::Is64BitOperatingSystem
        AvailableRAM_GB = [Math]::Round((Get-CimInstance Win32_ComputerSystem).TotalPhysicalMemory / 1GB, 2)
        FreeSpace_GB = [Math]::Round((Get-WmiObject -Class Win32_LogicalDisk -Filter "DeviceID='C:'").FreeSpace / 1GB, 2)
        AudioService = (Get-Service -Name "AudioSrv").Status
        WindowsDefender = $false
    }

    # Check Windows Defender
    try {
        $defenderStatus = Get-MpComputerStatus -ErrorAction SilentlyContinue
        $TestResults.SystemInfo.WindowsDefender = $defenderStatus.RealTimeProtectionEnabled
    } catch {
        $TestResults.SystemInfo.WindowsDefender = "Unknown"
    }

    Write-TestLog "System: $($TestResults.SystemInfo.OSVersion), RAM: $($TestResults.SystemInfo.AvailableRAM_GB)GB"

    # Validate minimum requirements
    $reqMet = $true
    if (-not $TestResults.SystemInfo.Architecture) {
        Write-TestLog "FAIL: 64-bit architecture required" "ERROR"
        $reqMet = $false
    }

    if ($TestResults.SystemInfo.AvailableRAM_GB -lt 4) {
        Write-TestLog "WARN: Less than 4GB RAM available ($($TestResults.SystemInfo.AvailableRAM_GB)GB)" "WARNING"
        $TestResults.Recommendations += "Recommend testing on system with 8GB+ RAM"
    }

    if ($TestResults.SystemInfo.FreeSpace_GB -lt 1) {
        Write-TestLog "FAIL: Insufficient disk space ($($TestResults.SystemInfo.FreeSpace_GB)GB free)" "ERROR"
        $reqMet = $false
    }

    return $reqMet
}

function Test-InstallerExecution {
    Write-TestLog "Testing installer execution..."

    $TestResults.InstallerTests = @{
        ExecutionResult = $false
        InstallTime_Seconds = 0
        RegistryEntries = $false
        Files = @{
            Executable = $false
            Config = $false
            HRTF = $false
            Documentation = $false
        }
        Shortcuts = @{
            StartMenu = $false
            Desktop = $false
        }
        Dependencies = @{
            VCRedist = $false
            OpenVR = $false
        }
    }

    # Execute installer silently
    $startTime = Get-Date
    try {
        Write-TestLog "Executing installer: $InstallerPath"
        $installProcess = Start-Process -FilePath $InstallerPath -ArgumentList "/S" -Wait -PassThru
        $endTime = Get-Date
        $TestResults.InstallerTests.InstallTime_Seconds = [Math]::Round(($endTime - $startTime).TotalSeconds, 2)

        if ($installProcess.ExitCode -eq 0) {
            $TestResults.InstallerTests.ExecutionResult = $true
            Write-TestLog "Installer completed successfully in $($TestResults.InstallerTests.InstallTime_Seconds) seconds"
        } else {
            Write-TestLog "Installer failed with exit code: $($installProcess.ExitCode)" "ERROR"
            return $false
        }
    } catch {
        Write-TestLog "Installer execution failed: $($_.Exception.Message)" "ERROR"
        return $false
    }

    # Check registry entries
    try {
        $uninstallKey = Get-ItemProperty "HKLM:\SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\VR Binaural Recorder" -ErrorAction SilentlyContinue
        if ($uninstallKey) {
            $TestResults.InstallerTests.RegistryEntries = $true
            Write-TestLog "Registry entries created successfully"
        } else {
            Write-TestLog "Registry entries not found" "ERROR"
        }
    } catch {
        Write-TestLog "Failed to check registry entries: $($_.Exception.Message)" "ERROR"
    }

    # Check installed files
    $installPath = "${env:ProgramFiles}\VR Binaural Recorder"

    $TestResults.InstallerTests.Files.Executable = Test-Path "$installPath\vr_binaural_recorder.exe"
    $TestResults.InstallerTests.Files.Config = Test-Path "$installPath\vr_binaural_config.json"
    $TestResults.InstallerTests.Files.HRTF = (Test-Path "$installPath\hrtf") -and (Get-ChildItem "$installPath\hrtf" -ErrorAction SilentlyContinue).Count -gt 0
    $TestResults.InstallerTests.Files.Documentation = (Test-Path "$installPath\README.md") -or (Test-Path "$installPath\LICENSE")

    # Check shortcuts
    $TestResults.InstallerTests.Shortcuts.StartMenu = Test-Path "$env:ProgramData\Microsoft\Windows\Start Menu\Programs\VR Binaural Recorder\VR Binaural Recorder.lnk"
    $TestResults.InstallerTests.Shortcuts.Desktop = Test-Path "$env:PUBLIC\Desktop\VR Binaural Recorder.lnk"

    # Log file check results
    foreach ($fileType in $TestResults.InstallerTests.Files.Keys) {
        $status = if ($TestResults.InstallerTests.Files[$fileType]) { "OK" } else { "MISSING" }
        Write-TestLog "File check - $fileType`: $status"
    }

    return $TestResults.InstallerTests.ExecutionResult
}

function Test-ApplicationFunctionality {
    Write-TestLog "Testing application functionality..."

    $TestResults.ApplicationTests = @{
        LaunchTest = $false
        ConfigurationTest = $false
        DependencyTest = $false
    }

    $installPath = "${env:ProgramFiles}\VR Binaural Recorder"
    $exePath = "$installPath\vr_binaural_recorder.exe"

    if (-not (Test-Path $exePath)) {
        Write-TestLog "Application executable not found: $exePath" "ERROR"
        return $false
    }

    # Test application launch (brief execution)
    try {
        Write-TestLog "Testing application launch..."
        $appProcess = Start-Process -FilePath $exePath -ArgumentList "--version" -Wait -PassThru -WindowStyle Hidden -ErrorAction SilentlyContinue

        if ($appProcess.ExitCode -eq 0 -or $appProcess.ExitCode -eq $null) {
            $TestResults.ApplicationTests.LaunchTest = $true
            Write-TestLog "Application launch test passed"
        } else {
            Write-TestLog "Application launch test failed with exit code: $($appProcess.ExitCode)" "ERROR"
        }
    } catch {
        Write-TestLog "Application launch test error: $($_.Exception.Message)" "WARNING"
        # Don't fail completely as some applications don't support --version
    }

    # Test configuration file
    $configPath = "$installPath\vr_binaural_config.json"
    if (Test-Path $configPath) {
        try {
            $config = Get-Content $configPath | ConvertFrom-Json
            $TestResults.ApplicationTests.ConfigurationTest = $true
            Write-TestLog "Configuration file validation passed"
        } catch {
            Write-TestLog "Configuration file validation failed: $($_.Exception.Message)" "ERROR"
        }
    }

    # Test dependencies (DLL files)
    $dllFiles = Get-ChildItem "$installPath\*.dll" -ErrorAction SilentlyContinue
    if ($dllFiles.Count -gt 0) {
        $TestResults.ApplicationTests.DependencyTest = $true
        Write-TestLog "Found $($dllFiles.Count) dependency files"
    } else {
        Write-TestLog "No dependency files found" "WARNING"
    }

    return $TestResults.ApplicationTests.LaunchTest -and $TestResults.ApplicationTests.ConfigurationTest
}

function Test-UninstallProcess {
    param([bool]$PerformUninstall = $true)

    if (-not $PerformUninstall) {
        Write-TestLog "Skipping uninstall test as requested"
        return $true
    }

    Write-TestLog "Testing uninstall process..."

    $TestResults.UninstallTests = @{
        UninstallResult = $false
        UninstallTime_Seconds = 0
        CleanupComplete = $false
        LeftoverFiles = @()
        LeftoverRegistry = $false
    }

    # Find uninstaller
    $installPath = "${env:ProgramFiles}\VR Binaural Recorder"
    $uninstallerPath = "$installPath\uninst.exe"

    if (-not (Test-Path $uninstallerPath)) {
        Write-TestLog "Uninstaller not found: $uninstallerPath" "ERROR"
        return $false
    }

    # Execute uninstaller
    $startTime = Get-Date
    try {
        Write-TestLog "Executing uninstaller..."
        $uninstallProcess = Start-Process -FilePath $uninstallerPath -ArgumentList "/S" -Wait -PassThru
        $endTime = Get-Date
        $TestResults.UninstallTests.UninstallTime_Seconds = [Math]::Round(($endTime - $startTime).TotalSeconds, 2)

        if ($uninstallProcess.ExitCode -eq 0) {
            $TestResults.UninstallTests.UninstallResult = $true
            Write-TestLog "Uninstaller completed successfully in $($TestResults.UninstallTests.UninstallTime_Seconds) seconds"
        } else {
            Write-TestLog "Uninstaller failed with exit code: $($uninstallProcess.ExitCode)" "ERROR"
        }
    } catch {
        Write-TestLog "Uninstaller execution failed: $($_.Exception.Message)" "ERROR"
    }

    # Wait a moment for file system operations to complete
    Start-Sleep -Seconds 2

    # Check for leftover files
    if (Test-Path $installPath) {
        $leftoverFiles = Get-ChildItem $installPath -Recurse -ErrorAction SilentlyContinue
        $TestResults.UninstallTests.LeftoverFiles = $leftoverFiles.FullName
        Write-TestLog "Found $($leftoverFiles.Count) leftover files" "WARNING"
    } else {
        $TestResults.UninstallTests.CleanupComplete = $true
        Write-TestLog "Installation directory cleaned successfully"
    }

    # Check for leftover registry entries
    try {
        $regKey = Get-ItemProperty "HKLM:\SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\VR Binaural Recorder" -ErrorAction SilentlyContinue
        if ($regKey) {
            $TestResults.UninstallTests.LeftoverRegistry = $true
            Write-TestLog "Leftover registry entries found" "WARNING"
        } else {
            Write-TestLog "Registry cleanup successful"
        }
    } catch {
        # Registry key doesn't exist - this is good
    }

    return $TestResults.UninstallTests.UninstallResult
}

function Generate-TestReport {
    Write-TestLog "Generating test report..."

    # Calculate overall result
    $TestResults.OverallResult = (
        $TestResults.InstallerTests.ExecutionResult -and
        $TestResults.InstallerTests.RegistryEntries -and
        $TestResults.InstallerTests.Files.Executable -and
        $TestResults.ApplicationTests.LaunchTest
    )

    # Generate recommendations
    if ($TestResults.SystemInfo.AvailableRAM_GB -lt 8) {
        $TestResults.Recommendations += "Consider testing with 8GB+ RAM for optimal performance validation"
    }

    if ($TestResults.SystemInfo.WindowsDefender -eq $true) {
        $TestResults.Recommendations += "Test with Windows Defender exclusions for VR Binaural Recorder directory"
    }

    if (-not $TestResults.ApplicationTests.DependencyTest) {
        $TestResults.Recommendations += "Validate all required DLL dependencies are included in installer"
    }

    if ($TestResults.UninstallTests.LeftoverFiles.Count -gt 0) {
        $TestResults.Recommendations += "Review uninstaller to ensure complete file cleanup"
    }

    # Save report
    $TestResults | ConvertTo-Json -Depth 10 | Out-File -FilePath $ReportPath -Encoding UTF8
    Write-TestLog "Test report saved to: $ReportPath"

    # Display summary
    Write-Host "`n======== INSTALLER TEST SUMMARY ========" -ForegroundColor Cyan
    Write-Host "Overall Result: $(if ($TestResults.OverallResult) { 'PASS' } else { 'FAIL' })" -ForegroundColor $(if ($TestResults.OverallResult) { 'Green' } else { 'Red' })
    Write-Host "Install Time: $($TestResults.InstallerTests.InstallTime_Seconds) seconds"
    if ($TestResults.UninstallTests.UninstallTime_Seconds -gt 0) {
        Write-Host "Uninstall Time: $($TestResults.UninstallTests.UninstallTime_Seconds) seconds"
    }

    if ($TestResults.Recommendations.Count -gt 0) {
        Write-Host "`nRecommendations:" -ForegroundColor Yellow
        foreach ($rec in $TestResults.Recommendations) {
            Write-Host "  • $rec" -ForegroundColor Yellow
        }
    }

    Write-Host "========================================`n" -ForegroundColor Cyan
}

# Main execution
Write-TestLog "Starting VR Binaural Recorder installer testing..."
Write-TestLog "Installer: $InstallerPath"

if (-not (Test-Path $InstallerPath)) {
    Write-TestLog "Installer file not found: $InstallerPath" "ERROR"
    exit 1
}

# Run tests
$systemOK = Test-SystemRequirements
$installOK = $false
$appOK = $false
$uninstallOK = $false

if ($systemOK) {
    $installOK = Test-InstallerExecution
    if ($installOK) {
        $appOK = Test-ApplicationFunctionality
        $uninstallOK = Test-UninstallProcess -PerformUninstall $UninstallAfterTest
    }
}

Generate-TestReport

# Exit with appropriate code
if ($TestResults.OverallResult) {
    Write-TestLog "All tests completed successfully!"
    exit 0
} else {
    Write-TestLog "Some tests failed. Check the report for details." "ERROR"
    exit 1
}