// openvr_stub.h - Minimal OpenVR header stubs for development
// This allows building without the full OpenVR SDK while maintaining the interface

#pragma once

#include <cstdint>

namespace vr {

// Minimal type definitions needed for compilation
typedef uint32_t TrackedDeviceIndex_t;
typedef uint64_t VROverlayHandle_t;

static const TrackedDeviceIndex_t k_unTrackedDeviceIndex_Hmd = 0;
static const TrackedDeviceIndex_t k_unMaxTrackedDeviceCount = 64;
static const TrackedDeviceIndex_t k_unTrackedDeviceIndexInvalid = 0xFFFFFFFF;
static const VROverlayHandle_t k_ulOverlayHandleInvalid = 0;

// Basic enums
enum EVRInitError {
    VRInitError_None = 0,
    VRInitError_Unknown = 1,
    VRInitError_Init_InstallationNotFound = 100,
    VRInitError_Init_InstallationCorrupt = 101,
    VRInitError_Init_VRClientDLLNotFound = 102,
    VRInitError_Init_FileNotFound = 103,
    VRInitError_Init_FactoryNotFound = 104,
    VRInitError_Init_InterfaceNotFound = 105,
    VRInitError_Init_InvalidInterface = 106,
    VRInitError_Init_UserConfigDirectoryInvalid = 107,
    VRInitError_Init_HmdNotFound = 108,
    VRInitError_Init_NotInitialized = 109,
    VRInitError_Init_PathRegistryNotFound = 110,
    VRInitError_Init_NoConfigPath = 111,
    VRInitError_Init_NoLogPath = 112,
    VRInitError_Init_PathRegistryNotWritable = 113,
    VRInitError_Init_AppInfoInitFailed = 114,
    VRInitError_Init_Retry = 115,
    VRInitError_Init_InitCanceledByUser = 116,
    VRInitError_Init_AnotherAppLaunching = 117,
    VRInitError_Init_SettingsInitFailed = 118,
    VRInitError_Init_ShuttingDown = 119,
    VRInitError_Init_TooManyObjects = 120,
    VRInitError_Init_NoServerForBackgroundApp = 121,
    VRInitError_Init_NotSupportedWithCompositor = 122,
    VRInitError_Init_NotAvailableToUtilityApps = 123,
    VRInitError_Init_Internal = 124,
    VRInitError_Init_HmdDriverIdIsNone = 125,
    VRInitError_Init_HmdNotFoundPresenceFailed = 126,
    VRInitError_Init_VRMonitorNotFound = 127,
    VRInitError_Init_VRMonitorStartupFailed = 128,
    VRInitError_Init_LowPowerWatchdogNotSupported = 129,
    VRInitError_Init_InvalidApplicationType = 130,
    VRInitError_Init_NotAvailableToWatchdogApps = 131,
    VRInitError_Init_WatchdogDisabledInSettings = 132,
    VRInitError_Init_VRDashboardNotFound = 133,
    VRInitError_Init_VRDashboardStartupFailed = 134,
    VRInitError_Init_VRHomeNotFound = 135,
    VRInitError_Init_VRHomeStartupFailed = 136,
    VRInitError_Init_RebootingBusy = 137,
    VRInitError_Init_FirmwareUpdateBusy = 138,
    VRInitError_Init_FirmwareRecoveryBusy = 139,
    VRInitError_Init_USBServiceBusy = 140,
    VRInitError_Init_VRWebHelperStartupFailed = 141,
    VRInitError_Init_TrackerManagerInitFailed = 142,
    VRInitError_Init_AlreadyRunning = 143,
    VRInitError_Init_FailedForVrMonitor = 144,
    VRInitError_Init_PropertyManagerInitFailed = 145,
    VRInitError_Init_WebServerFailed = 146,
    VRInitError_Init_IllegalTypeTransition = 147,
    VRInitError_Init_MismatchedRuntimes = 148,
    VRInitError_Init_InvalidProcessId = 149,
    VRInitError_Init_VRServiceStartupFailed = 150,
    VRInitError_Init_InsufficientPermissions = 151,
    VRInitError_Init_ServerNotReachable = 152,
    VRInitError_Init_CouldNotStartServer = 153,
    VRInitError_Init_Compositor_Failed = 400,
    VRInitError_Init_Compositor_D3D11HardwareRequired = 401,
    VRInitError_Init_Compositor_FirmwareRequiresUpdate = 402,
    VRInitError_Init_Compositor_OverlayInitFailed = 403,
    VRInitError_Init_Compositor_ScreenshotsInitFailed = 404,
    VRInitError_Init_Compositor_UnableToCreateDevice = 405,
    VRInitError_Init_Compositor_SharedStateIsNull = 406,
    VRInitError_Init_Compositor_NotificationManagerIsNull = 407,
    VRInitError_Init_Compositor_ResourceManagerClientIsNull = 408,
    VRInitError_Init_Compositor_MessageOverlaySharedStateInitFailure = 409,
    VRInitError_Init_Compositor_PropertiesInterfaceIsNull = 410,
    VRInitError_Init_Compositor_CreateFullscreenWindowFailed = 411,
    VRInitError_Init_Compositor_SettingsInterfaceIsNull = 412,
    VRInitError_Init_Compositor_FailedToShowWindow = 413,
    VRInitError_Init_Compositor_DistortInterfaceIsNull = 414,
    VRInitError_Init_Compositor_DisplayFrequencyFailure = 415,
    VRInitError_Init_Compositor_RendererInitializationFailed = 416,
    VRInitError_Init_Compositor_DXGIFactoryInterfaceIsNull = 417,
    VRInitError_Init_Compositor_DXGIFactoryCreateFailed = 418,
    VRInitError_Init_Compositor_DXGIFactoryQueryFailed = 419,
    VRInitError_Init_Compositor_InvalidAdapterDesktop = 420,
    VRInitError_Init_Compositor_InvalidHmdAttachment = 421,
    VRInitError_Init_Compositor_InvalidOutputDesktop = 422,
    VRInitError_Init_Compositor_InvalidDeviceProvided = 423,
    VRInitError_Init_Compositor_D3D11RendererInitializationFailed = 424,
    VRInitError_Init_Compositor_FailedToFindDisplayMode = 425,
    VRInitError_Init_Compositor_FailedToCreateSwapChain = 426,
    VRInitError_Init_Compositor_FailedToGetBackBuffer = 427,
    VRInitError_Init_Compositor_FailedToCreateRenderTarget = 428,
    VRInitError_Init_Compositor_FailedToCreateDXGI2SwapChain = 429,
    VRInitError_Init_Compositor_FailedtoGetDXGI2BackBuffer = 430,
    VRInitError_Init_Compositor_FailedToCreateDXGI2RenderTarget = 431,
    VRInitError_Init_Compositor_FailedToGetDXGIDeviceInterface = 432,
    VRInitError_Init_Compositor_SelectDisplayMode = 433,
    VRInitError_Init_Compositor_FailedToCreateNvAPIRenderTargets = 434,
    VRInitError_Init_Compositor_NvAPISetDisplayMode = 435,
    VRInitError_Init_Compositor_FailedToCreateDirectModeDisplay = 436,
    VRInitError_Init_Compositor_InvalidHmdPropertyContainer = 437,
    VRInitError_Init_Compositor_UpdateDisplayFrequency = 438,
    VRInitError_Init_Compositor_CreateRasterizerState = 439,
    VRInitError_Init_Compositor_CreateWireframeRasterizerState = 440,
    VRInitError_Init_Compositor_CreateSamplerState = 441,
    VRInitError_Init_Compositor_CreateClampToBorderSamplerState = 442,
    VRInitError_Init_Compositor_CreateAnisoSamplerState = 443,
    VRInitError_Init_Compositor_CreateOverlaySamplerState = 444,
    VRInitError_Init_Compositor_CreatePanoramaSamplerState = 445,
    VRInitError_Init_Compositor_CreateFontSamplerState = 446,
    VRInitError_Init_Compositor_CreateNoBlendState = 447,
    VRInitError_Init_Compositor_CreateBlendState = 448,
    VRInitError_Init_Compositor_CreateAlphaBlendState = 449,
    VRInitError_Init_Compositor_CreateBlendStateMaskR = 450,
    VRInitError_Init_Compositor_CreateBlendStateMaskG = 451,
    VRInitError_Init_Compositor_CreateBlendStateMaskB = 452,
    VRInitError_Init_Compositor_CreateDepthStencilState = 453,
    VRInitError_Init_Compositor_CreateDepthStencilStateNoWrite = 454,
    VRInitError_Init_Compositor_CreateDepthStencilStateNoDepth = 455,
    VRInitError_Init_Compositor_CreateFlushTexture = 456,
    VRInitError_Init_Compositor_CreateDistortionSurfaces = 457,
    VRInitError_Init_Compositor_CreateConstantBuffer = 458,
    VRInitError_Init_Compositor_CreateHmdPoseConstantBuffer = 459,
    VRInitError_Init_Compositor_CreateHmdPoseStagingConstantBuffer = 460,
    VRInitError_Init_Compositor_CreateSharedFrameInfoConstantBuffer = 461,
    VRInitError_Init_Compositor_CreateOverlayConstantBuffer = 462,
    VRInitError_Init_Compositor_CreateSceneTextureIndexConstantBuffer = 463,
    VRInitError_Init_Compositor_CreateReadableSceneTextureIndexConstantBuffer = 464,
    VRInitError_Init_Compositor_CreateLayerGraphicsTextureIndexConstantBuffer = 465,
    VRInitError_Init_Compositor_CreateLayerComputeTextureIndexConstantBuffer = 466,
    VRInitError_Init_Compositor_CreateLayerComputeSceneTextureIndexConstantBuffer = 467,
    VRInitError_Init_Compositor_CreateComputeHmdPoseConstantBuffer = 468,
    VRInitError_Init_Compositor_CreateGeomConstantBuffer = 469,
    VRInitError_Init_Compositor_CreatePanelMaskConstantBuffer = 470,
    VRInitError_Init_Compositor_CreatePixelSimUBO = 471,
    VRInitError_Init_Compositor_CreateMSAARenderTextures = 472,
    VRInitError_Init_Compositor_CreateResolveRenderTextures = 473,
    VRInitError_Init_Compositor_CreateComputeResolveRenderTextures = 474,
    VRInitError_Init_Compositor_CreateDriverDirectModeResolveTextures = 475,
    VRInitError_Init_Compositor_OpenDriverDirectModeResolveTextures = 476,
    VRInitError_Init_Compositor_CreateFallbackSyncTexture = 477,
    VRInitError_Init_Compositor_ShareFallbackSyncTexture = 478,
    VRInitError_Init_Compositor_CreateOverlayIndexBuffer = 479,
    VRInitError_Init_Compositor_CreateOverlayVertexBuffer = 480,
    VRInitError_Init_Compositor_CreateTextVertexBuffer = 481,
    VRInitError_Init_Compositor_CreateTextIndexBuffer = 482,
    VRInitError_Init_Compositor_CreateMirrorTextures = 483,
    VRInitError_Init_Compositor_CreateLastFrameRenderTexture = 484,
    VRInitError_Init_Compositor_CreateMirrorOverlay = 485,
    VRInitError_Init_Compositor_FailedToCreateVirtualDisplayBackbuffer = 486,
    VRInitError_Init_Compositor_DisplayModeNotSupported = 487,
    VRInitError_Init_Compositor_CreateOverlayInvalidCall = 488,
    VRInitError_Init_Compositor_CreateOverlayAlreadyInitialized = 489,
    VRInitError_Init_Compositor_FailedToCreateMailbox = 490,
    VRInitError_VendorSpecific_UnableToConnectToOculusRuntime = 1000,
    VRInitError_VendorSpecific_WindowsNotInDevMode = 1001,
    VRInitError_VendorSpecific_HmdFound_CantOpenDevice = 1101,
    VRInitError_VendorSpecific_HmdFound_UnableToRequestConfigStart = 1102,
    VRInitError_VendorSpecific_HmdFound_NoStoredConfig = 1103,
    VRInitError_VendorSpecific_HmdFound_ConfigTooBig = 1104,
    VRInitError_VendorSpecific_HmdFound_ConfigTooSmall = 1105,
    VRInitError_VendorSpecific_HmdFound_UnableToInitZLib = 1106,
    VRInitError_VendorSpecific_HmdFound_CantReadFirmwareVersion = 1107,
    VRInitError_VendorSpecific_HmdFound_UnableToSendUserDataStart = 1108,
    VRInitError_VendorSpecific_HmdFound_UnableToGetUserDataStart = 1109,
    VRInitError_VendorSpecific_HmdFound_UnableToGetUserDataNext = 1110,
    VRInitError_VendorSpecific_HmdFound_UserDataAddressRange = 1111,
    VRInitError_VendorSpecific_HmdFound_UserDataError = 1112,
    VRInitError_VendorSpecific_HmdFound_ConfigFailedSanityCheck = 1113,
    VRInitError_VendorSpecific_OculusRuntimeBadInstall = 1114,
    VRInitError_Steam_SteamInstallationNotFound = 2000,
    VRInitError_LastError
};

enum ETrackedDeviceClass {
    TrackedDeviceClass_Invalid = 0,
    TrackedDeviceClass_HMD = 1,
    TrackedDeviceClass_Controller = 2,
    TrackedDeviceClass_GenericTracker = 3,
    TrackedDeviceClass_TrackingReference = 4,
    TrackedDeviceClass_DisplayRedirect = 5,
    TrackedDeviceClass_Max
};

enum ETrackedControllerRole {
    TrackedControllerRole_Invalid = 0,
    TrackedControllerRole_LeftHand = 1,
    TrackedControllerRole_RightHand = 2
};

enum ETrackedDeviceProperty {
    Prop_Invalid = 0,
    Prop_TrackingSystemName_String = 1000,
    Prop_ModelNumber_String = 1001,
    Prop_SerialNumber_String = 1002,
    Prop_RenderModelName_String = 1003,
    Prop_WillDriftInYaw_Bool = 1004,
    Prop_ManufacturerName_String = 1005,
    Prop_TrackingFirmwareVersion_String = 1006,
    Prop_HardwareRevision_String = 1007,
    Prop_AllWirelessDongleDescriptions_String = 1008,
    Prop_ConnectedWirelessDongle_String = 1009,
    Prop_DeviceIsWireless_Bool = 1010,
    Prop_DeviceIsCharging_Bool = 1011,
    Prop_DeviceBatteryPercentage_Float = 1012,
    Prop_StatusDisplayTransform_Matrix34 = 1013,
    Prop_Firmware_UpdateAvailable_Bool = 1014,
    Prop_Firmware_ManualUpdate_Bool = 1015,
    Prop_Firmware_ManualUpdateURL_String = 1016,
    Prop_HardwareRevision_Uint64 = 1017,
    Prop_FirmwareVersion_Uint64 = 1018,
    Prop_FPGAVersion_Uint64 = 1019,
    Prop_VRCVersion_Uint64 = 1020,
    Prop_RadioVersion_Uint64 = 1021,
    Prop_DongleVersion_Uint64 = 1022,
    Prop_BlockServerShutdown_Bool = 1023,
    Prop_CanUnifyCoordinateSystemWithHmd_Bool = 1024,
    Prop_ContainsProximitySensor_Bool = 1025,
    Prop_DeviceProvidesBatteryStatus_Bool = 1026,
    Prop_DeviceCanPowerOff_Bool = 1027,
    Prop_Firmware_ProgrammingTarget_String = 1028,
    Prop_DeviceClass_Int32 = 1029,
    Prop_HasCamera_Bool = 1030,
    Prop_DriverVersion_String = 1031,
    Prop_Firmware_ForceUpdateRequired_Bool = 1032,
    Prop_ViveSystemButtonFixRequired_Bool = 1033,
    Prop_ParentDriver_Uint64 = 1034,
    Prop_ResourceRoot_String = 1035,
    Prop_RegisteredDeviceType_String = 1036,
    Prop_InputProfilePath_String = 1037,
    Prop_NeverTracked_Bool = 1038,
    Prop_NumCameras_Int32 = 1039,
    Prop_CameraFrameLayout_Int32 = 1040,
    Prop_CameraStreamFormat_Int32 = 1041,
    Prop_AdditionalDeviceSettingsPath_String = 1042,
    Prop_Identifiable_Bool = 1043,
    Prop_BootloaderVersion_Uint64 = 1044,
    Prop_AdditionalSystemReportData_String = 1045,
    Prop_CompositeFirmwareVersion_String = 1046,
    Prop_Firmware_RemindUpdate_Bool = 1047,
    Prop_PeripheralApplicationVersion_Uint64 = 1048,
    Prop_ManufacturerSerialNumber_String = 1049,
    Prop_ComputedSerialNumber_String = 1050,
    Prop_EstimatedDeviceFirstUseTime_Int32 = 1051,
    Prop_DevicePowerUsage_Float = 1052,
    Prop_IgnoreMotionForStandby_Bool = 1053,
    Prop_ReportsTimeSinceVSync_Bool = 2000,
    Prop_SecondsFromVsyncToPhotons_Float = 2001,
    Prop_DisplayFrequency_Float = 2002,
    Prop_UserIpdMeters_Float = 2003,
    Prop_CurrentUniverseId_Uint64 = 2004,
    Prop_PreviousUniverseId_Uint64 = 2005,
    Prop_DisplayFirmwareVersion_Uint64 = 2006,
    Prop_IsOnDesktop_Bool = 2007,
    Prop_DisplayMCType_Int32 = 2008,
    Prop_DisplayMCOffset_Float = 2009,
    Prop_DisplayMCScale_Float = 2010,
    Prop_DisplayGCBlackClamp_Float = 2011,
    Prop_EdidVendorID_Int32 = 2012,
    Prop_DisplayMCImageLeft_String = 2013,
    Prop_DisplayMCImageRight_String = 2014,
    Prop_DisplayGCPrescale_Float = 2015,
    Prop_DisplayGCType_Int32 = 2016,
    Prop_LensCenterLeftU_Float = 2017,
    Prop_LensCenterLeftV_Float = 2018,
    Prop_LensCenterRightU_Float = 2019,
    Prop_LensCenterRightV_Float = 2020,
    Prop_UserHeadToEyeDepthMeters_Float = 2021,
    Prop_CameraFirmwareVersion_Uint64 = 2022,
    Prop_CameraFirmwareDescription_String = 2023,
    Prop_DisplayFPGAVersion_Uint64 = 2024,
    Prop_DisplayBootloaderVersion_Uint64 = 2025,
    Prop_DisplayHardwareVersion_Uint64 = 2026,
    Prop_AudioFirmwareVersion_Uint64 = 2027,
    Prop_CameraCompatibilityMode_Int32 = 2028,
    Prop_ScreenshotHorizontalFieldOfViewDegrees_Float = 2029,
    Prop_ScreenshotVerticalFieldOfViewDegrees_Float = 2030,
    Prop_DisplaySuppressed_Bool = 2031,
    Prop_DisplayAllowNightMode_Bool = 2032,
    Prop_DisplayMCImageWidth_Int32 = 2033,
    Prop_DisplayMCImageHeight_Int32 = 2034,
    Prop_DisplayMCImageNumChannels_Int32 = 2035,
    Prop_DisplayMCImageData_Binary = 2036,
    Prop_SecondsFromPhotonsToVblank_Float = 2037,
    Prop_DriverDirectModeSendsVsyncEvents_Bool = 2038,
    Prop_DisplayDebugMode_Bool = 2039,
    Prop_GraphicsAdapterLuid_Uint64 = 2040,
    Prop_DriverProvidedChaperonePath_String = 2048,
    Prop_ExpectedTrackingReferenceCount_Int32 = 2049,
    Prop_ExpectedControllerCount_Int32 = 2050,
    Prop_NamedIconPathControllerLeftDeviceOff_String = 2051,
    Prop_NamedIconPathControllerRightDeviceOff_String = 2052,
    Prop_NamedIconPathTrackingReferenceDeviceOff_String = 2053,
    Prop_DoNotApplyPrediction_Bool = 2054,
    Prop_CameraToHeadTransforms_Matrix34_Array = 2055,
    Prop_DistortionMeshResolution_Int32 = 2056,
    Prop_DriverIsDrawingControllers_Bool = 2057,
    Prop_DriverRequestsApplicationPause_Bool = 2058,
    Prop_DriverRequestsReducedRendering_Bool = 2059,
    Prop_MinimumIpdStepMeters_Float = 2060,
    Prop_AudioBridgeFirmwareVersion_Uint64 = 2061,
    Prop_ImageBridgeFirmwareVersion_Uint64 = 2062,
    Prop_ImuToHeadTransform_Matrix34 = 2063,
    Prop_ImuFactoryGyroBias_Vector3 = 2064,
    Prop_ImuFactoryGyroScale_Vector3 = 2065,
    Prop_ImuFactoryAccelerometerBias_Vector3 = 2066,
    Prop_ImuFactoryAccelerometerScale_Vector3 = 2067,
    Prop_ConfigurationIncludesLighthouse20Features_Bool = 2069,
    Prop_AdditionalRadioFeatures_Uint64 = 2070,
    Prop_CameraWhiteBalance_Vector4_Array = 2071,
    Prop_CameraDistortionFunction_Int32_Array = 2072,
    Prop_CameraDistortionCoefficients_Float_Array = 2073,
    Prop_ExpectedControllerType_String = 2074,
    Prop_HmdTrackingStyle_Int32 = 2075,
    Prop_DriverProvidedChaperoneVisibility_Bool = 2076,
    Prop_HmdColumnCorrectionSettingPrefix_String = 2077,
    Prop_CameraSupportsCompatibilityModes_Bool = 2078,
    Prop_SupportsRoomViewDepthProjection_Bool = 2079,
    Prop_DisplayAvailableFrameRates_Float_Array = 2080,
    Prop_DisplaySupportsMultipleFramerates_Bool = 2081,
    Prop_DisplayColorMultLeft_Vector3 = 2082,
    Prop_DisplayColorMultRight_Vector3 = 2083,
    Prop_DisplaySupportsRuntimeFramerateChange_Bool = 2084,
    Prop_DisplaySupportsAnalogGain_Bool = 2085,
    Prop_DisplayMinAnalogGain_Float = 2086,
    Prop_DisplayMaxAnalogGain_Float = 2087,
    Prop_CameraExposureTime_Float = 2088,
    Prop_CameraGlobalGain_Float = 2089,
    Prop_DashboardScale_Float = 2091,
    Prop_IpdUIRangeMinMeters_Float = 2100,
    Prop_IpdUIRangeMaxMeters_Float = 2101,
    Prop_Hmd_SupportsHDCP14LegacyCompat_Bool = 2102,
    Prop_Hmd_SupportsMicMonitoring_Bool = 2103,
    Prop_Hmd_SupportsDisplayPortTrainingMode_Bool = 2104,
    Prop_Hmd_SupportsRoomViewDirect_Bool = 2105,
    Prop_Hmd_SupportsAppThrottling_Bool = 2106,
    Prop_Hmd_SupportsGpuBusMonitoring_Bool = 2107,
    Prop_AttachedDeviceId_String = 3000,
    Prop_SupportedButtons_Uint64 = 3001,
    Prop_Axis0Type_Int32 = 3002,
    Prop_Axis1Type_Int32 = 3003,
    Prop_Axis2Type_Int32 = 3004,
    Prop_Axis3Type_Int32 = 3005,
    Prop_Axis4Type_Int32 = 3006,
    Prop_ControllerRoleHint_Int32 = 3007,
    Prop_FieldOfViewLeftDegrees_Float = 4000,
    Prop_FieldOfViewRightDegrees_Float = 4001,
    Prop_FieldOfViewTopDegrees_Float = 4002,
    Prop_FieldOfViewBottomDegrees_Float = 4003,
    Prop_TrackingRangeMinimumMeters_Float = 4004,
    Prop_TrackingRangeMaximumMeters_Float = 4005,
    Prop_ModeLabel_String = 4006,
    Prop_CanWirelessIdentify_Bool = 4007,
    Prop_Nonce_Int32 = 4008,
    Prop_IconPathName_String = 5000,
    Prop_NamedIconPathDeviceOff_String = 5001,
    Prop_NamedIconPathDeviceSearching_String = 5002,
    Prop_NamedIconPathDeviceSearchingAlert_String = 5003,
    Prop_NamedIconPathDeviceReady_String = 5004,
    Prop_NamedIconPathDeviceReadyAlert_String = 5005,
    Prop_NamedIconPathDeviceNotReady_String = 5006,
    Prop_NamedIconPathDeviceStandby_String = 5007,
    Prop_NamedIconPathDeviceAlertLow_String = 5008,
    Prop_NamedIconPathDeviceStandbyAlert_String = 5009,
    Prop_DisplayHiddenArea_Binary_Start = 5100,
    Prop_DisplayHiddenArea_Binary_End = 5150,
    Prop_ParentContainer = 5151,
    Prop_OverrideContainer_Uint64 = 5152,
    Prop_UserConfigPath_String = 6000,
    Prop_InstallPath_String = 6001,
    Prop_HasDisplayComponent_Bool = 6002,
    Prop_HasControllerComponent_Bool = 6003,
    Prop_HasCameraComponent_Bool = 6004,
    Prop_HasDriverDirectModeComponent_Bool = 6005,
    Prop_HasVirtualDisplayComponent_Bool = 6006,
    Prop_HasSpatialAnchorsSupport_Bool = 6007,
    Prop_SupportsXrTextureSets_Bool = 6008,
    Prop_ControllerType_String = 7000,
    Prop_ControllerHandSelectionPriority_Int32 = 7002,
    Prop_VendorSpecific_Reserved_Start = 10000,
    Prop_VendorSpecific_Reserved_End = 10999,
    Prop_TrackedDeviceProperty_Max = 1000000
};

enum EVRApplicationType {
    VRApplication_Other = 0,
    VRApplication_Scene = 1,
    VRApplication_Overlay = 2,
    VRApplication_Background = 3,
    VRApplication_Utility = 4,
    VRApplication_VRMonitor = 5,
    VRApplication_SteamWatchdog = 6,
    VRApplication_Bootstrapper = 7,
    VRApplication_WebHelper = 8,
    VRApplication_OpenXRInstance = 9,
    VRApplication_OpenXRScene = 10,
    VRApplication_OpenXROverlay = 11,
    VRApplication_Prism = 12,
    VRApplication_Max
};

enum ETrackingUniverseOrigin {
    TrackingUniverseSeated = 0,
    TrackingUniverseStanding = 1,
    TrackingUniverseRawAndUncalibrated = 2
};

enum EVROverlayError {
    VROverlayError_None = 0,
    VROverlayError_UnknownOverlay = 10,
    VROverlayError_InvalidHandle = 11,
    VROverlayError_PermissionDenied = 12,
    VROverlayError_OverlayLimitExceeded = 13,
    VROverlayError_WrongVisibilityType = 14,
    VROverlayError_KeyTooLong = 15,
    VROverlayError_NameTooLong = 16,
    VROverlayError_KeyInUse = 17,
    VROverlayError_WrongTransformType = 18,
    VROverlayError_InvalidTrackedDevice = 19,
    VROverlayError_InvalidParameter = 20,
    VROverlayError_ThumbnailCantBeDestroyed = 21,
    VROverlayError_ArrayTooSmall = 22,
    VROverlayError_RequestFailed = 23,
    VROverlayError_InvalidTexture = 24,
    VROverlayError_UnableToLoadFile = 25,
    VROverlayError_KeyboardAlreadyInUse = 26,
    VROverlayError_NoNeighbor = 27,
    VROverlayError_TooManyMaskPrimitives = 29,
    VROverlayError_BadMaskPrimitive = 30,
    VROverlayError_TextureAlreadyLocked = 31,
    VROverlayError_TextureLockCapacityReached = 32,
    VROverlayError_TextureNotLocked = 33
};

enum VRMouseButton {
    VRMouseButton_Left = 0x0001,
    VRMouseButton_Right = 0x0002,
    VRMouseButton_Middle = 0x0004
};

enum EVREventType {
    VREvent_None = 0,
    VREvent_TrackedDeviceActivated = 100,
    VREvent_TrackedDeviceDeactivated = 101,
    VREvent_TrackedDeviceUpdated = 102,
    VREvent_TrackedDeviceUserInteractionStarted = 103,
    VREvent_TrackedDeviceUserInteractionEnded = 104,
    VREvent_IpdChanged = 105,
    VREvent_EnterStandbyMode = 106,
    VREvent_LeaveStandbyMode = 107,
    VREvent_TrackedDeviceRoleChanged = 108,
    VREvent_WatchdogWakeUpRequested = 109,
    VREvent_LensDistortionChanged = 110,
    VREvent_PropertyChanged = 111,
    VREvent_WirelessDisconnect = 112,
    VREvent_WirelessReconnect = 113,
    VREvent_ButtonPress = 200,
    VREvent_ButtonUnpress = 201,
    VREvent_ButtonTouch = 202,
    VREvent_ButtonUntouch = 203,
    VREvent_Modal_Cancel = 257,
    VREvent_MouseMove = 300,
    VREvent_MouseButtonDown = 301,
    VREvent_MouseButtonUp = 302,
    VREvent_FocusEnter = 303,
    VREvent_FocusLeave = 304,
    VREvent_ScrollDiscrete = 305,
    VREvent_TouchPadMove = 306,
    VREvent_OverlayFocusChanged = 307,
    VREvent_ReloadOverlays = 308,
    VREvent_ScrollSmooth = 309,
    VREvent_LockMousePosition = 310,
    VREvent_UnlockMousePosition = 311,
    VREvent_InputFocusCaptured = 400,
    VREvent_InputFocusReleased = 401,
    VREvent_SceneApplicationChanged = 404,
    VREvent_SceneFocusChanged = 405,
    VREvent_InputCaptureChanged = 406,
    VREvent_SceneApplicationUsingWrongGraphicsAdapter = 408,
    VREvent_ActionBindingReloaded = 409,
    VREvent_HideRenderModels = 410,
    VREvent_ShowRenderModels = 411,
    VREvent_SceneApplicationStateChanged = 412,
    VREvent_ConsoleOpened = 420,
    VREvent_ConsoleClosed = 421,
    VREvent_OverlayShown = 500,
    VREvent_OverlayHidden = 501,
    VREvent_DashboardActivated = 502,
    VREvent_DashboardDeactivated = 503,
    VREvent_DashboardRequested = 505,
    VREvent_ResetDashboard = 506,
    VREvent_ImageLoaded = 508,
    VREvent_ShowKeyboard = 509,
    VREvent_HideKeyboard = 510,
    VREvent_OverlayGamepadFocusGained = 511,
    VREvent_OverlayGamepadFocusLost = 512,
    VREvent_OverlaySharedTextureChanged = 513,
    VREvent_ScreenshotTriggered = 516,
    VREvent_ImageFailed = 517,
    VREvent_DashboardOverlayCreated = 518,
    VREvent_SwitchGamepadFocus = 519,
    VREvent_RequestScreenshot = 520,
    VREvent_ScreenshotTaken = 521,
    VREvent_ScreenshotFailed = 522,
    VREvent_SubmitScreenshotToDashboard = 523,
    VREvent_ScreenshotProgressToDashboard = 524,
    VREvent_PrimaryDashboardDeviceChanged = 525,
    VREvent_RoomViewShown = 526,
    VREvent_RoomViewHidden = 527,
    VREvent_ShowUI = 528,
    VREvent_ShowDevTools = 529,
    VREvent_DesktopViewUpdating = 530,
    VREvent_DesktopViewReady = 531,
    VREvent_StartDashboard = 532,
    VREvent_ElevatePermissions = 533,
    VREvent_OverlaysClosed = 534,
    VREvent_DashboardThumbChanged = 535,
    VREvent_DesktopMightBeVisible = 536,
    VREvent_DesktopMightBeHidden = 537,
    VREvent_Notification_Shown = 600,
    VREvent_Notification_Hidden = 601,
    VREvent_Notification_BeginInteraction = 602,
    VREvent_Notification_Destroyed = 603,
    VREvent_Quit = 700,
    VREvent_ProcessQuit = 701,
    VREvent_QuitAcknowledged = 702,
    VREvent_DriverRequestedQuit = 703,
    VREvent_RestartRequested = 704,
    VREvent_ChaperoneDataHasChanged = 800,
    VREvent_ChaperoneUniverseHasChanged = 801,
    VREvent_ChaperoneTempDataHasChanged = 802,
    VREvent_ChaperoneSettingsHaveChanged = 803,
    VREvent_SeatedZeroPoseReset = 804,
    VREvent_ChaperoneFlushCache = 805,
    VREvent_ChaperoneRoomSetupStarting = 806,
    VREvent_ChaperoneRoomSetupFinished = 807,
    VREvent_StandingZeroPoseReset = 808,
    VREvent_AudioSettingsHaveChanged = 820,
    VREvent_BackgroundSettingHasChanged = 850,
    VREvent_CameraSettingsHaveChanged = 851,
    VREvent_ReprojectionSettingHasChanged = 852,
    VREvent_ModelSkinSettingsHaveChanged = 853,
    VREvent_EnvironmentSettingsHaveChanged = 854,
    VREvent_PowerSettingsHaveChanged = 855,
    VREvent_EnableHomeAppSettingsHaveChanged = 856,
    VREvent_SteamVRSectionSettingChanged = 857,
    VREvent_LighthouseSectionSettingChanged = 858,
    VREvent_NullSectionSettingChanged = 859,
    VREvent_UserInterfaceSectionSettingChanged = 860,
    VREvent_NotificationsSectionSettingChanged = 861,
    VREvent_KeyboardSectionSettingChanged = 862,
    VREvent_PerfSectionSettingChanged = 863,
    VREvent_DashboardSectionSettingChanged = 864,
    VREvent_WebInterfaceSectionSettingChanged = 865,
    VREvent_TrackersSectionSettingChanged = 866,
    VREvent_LastKnownSectionSettingChanged = 867,
    VREvent_DismissedWarningsSectionSettingChanged = 868,
    VREvent_GpuSpeedSectionSettingChanged = 869,
    VREvent_WindowsMixedRealitySectionSettingChanged = 870,
    VREvent_OtherSectionSettingChanged = 871,
    VREvent_AnyDriverSettingsChanged = 872,
    VREvent_StatusUpdate = 900,
    VREvent_WebInterface_InstallDriverRequested = 950,
    VREvent_MCImageUpdated = 1000,
    VREvent_FirmwareUpdateStarted = 1100,
    VREvent_FirmwareUpdateFinished = 1101,
    VREvent_KeyboardClosed = 1200,
    VREvent_KeyboardCharInput = 1201,
    VREvent_KeyboardDone = 1202,
    VREvent_ApplicationListUpdated = 1303,
    VREvent_ApplicationMimeTypeLoad = 1304,
    VREvent_ProcessConnected = 1306,
    VREvent_ProcessDisconnected = 1307,
    VREvent_Compositor_ChaperoneBoundsShown = 1410,
    VREvent_Compositor_ChaperoneBoundsHidden = 1411,
    VREvent_Compositor_DisplayDisconnected = 1412,
    VREvent_Compositor_DisplayReconnected = 1413,
    VREvent_Compositor_HDCPError = 1414,
    VREvent_Compositor_ApplicationNotResponding = 1415,
    VREvent_Compositor_ApplicationResumed = 1416,
    VREvent_Compositor_OutOfVideoMemory = 1417,
    VREvent_Compositor_DisplayModeNotSupported = 1418,
    VREvent_Compositor_StageOverrideReady = 1419,
    VREvent_Compositor_RequestDisconnectReconnect = 1420,
    VREvent_TrackedCamera_StartVideoStream = 1500,
    VREvent_TrackedCamera_StopVideoStream = 1501,
    VREvent_TrackedCamera_PauseVideoStream = 1502,
    VREvent_TrackedCamera_ResumeVideoStream = 1503,
    VREvent_TrackedCamera_EditingSurfaceChanged = 1550,
    VREvent_PerformanceTest_EnableCapture = 1600,
    VREvent_PerformanceTest_DisableCapture = 1601,
    VREvent_PerformanceTest_FidelityLevel = 1602,
    VREvent_MessageOverlay_Closed = 1650,
    VREvent_MessageOverlayCloseRequested = 1651,
    VREvent_Input_HapticVibration = 1700,
    VREvent_Input_BindingLoadFailed = 1701,
    VREvent_Input_BindingLoadSuccessful = 1702,
    VREvent_Input_ActionManifestReloaded = 1703,
    VREvent_Input_ActionManifestLoadFailed = 1704,
    VREvent_Input_ProgressUpdate = 1705,
    VREvent_Input_TrackerActivated = 1706,
    VREvent_Input_BindingsUpdated = 1707,
    VREvent_Input_BindingSubscriptionChanged = 1708,
    VREvent_SpatialAnchors_PoseUpdated = 1800,
    VREvent_SpatialAnchors_DescriptorUpdated = 1801,
    VREvent_SpatialAnchors_RequestPoseUpdate = 1802,
    VREvent_SpatialAnchors_RequestDescriptorUpdate = 1803,
    VREvent_SystemReport_Started = 1900,
    VREvent_Monitor_ShowHeadsetView = 2000,
    VREvent_Monitor_HideHeadsetView = 2001,
    VREvent_Audio_SetSpeakersVolume = 2100,
    VREvent_Audio_SetSpeakersMute = 2101,
    VREvent_Audio_SetMicrophoneVolume = 2102,
    VREvent_Audio_SetMicrophoneMute = 2103,
    VREvent_VendorSpecific_Reserved_Start = 10000,
    VREvent_VendorSpecific_Reserved_End = 19999
};

enum EVROverlayIntersectionMaskPrimitiveType {
    OverlayIntersectionPrimitiveType_Rectangle,
    OverlayIntersectionPrimitiveType_Circle
};

enum EVRRenderModelError {
    VRRenderModelError_None = 0,
    VRRenderModelError_Loading = 100,
    VRRenderModelError_NotSupported = 200,
    VRRenderModelError_InvalidArg = 300,
    VRRenderModelError_InvalidModel = 301,
    VRRenderModelError_NoShapes = 302,
    VRRenderModelError_MultipleShapes = 303,
    VRRenderModelError_TooManyVertices = 304,
    VRRenderModelError_MultipleTextures = 305,
    VRRenderModelError_BufferTooSmall = 306,
    VRRenderModelError_NotEnoughNormals = 307,
    VRRenderModelError_NotEnoughTexCoords = 308,
    VRRenderModelError_InvalidTexture = 400
};

enum ETextureType {
    TextureType_Invalid = -1,
    TextureType_DirectX = 0,
    TextureType_OpenGL = 1,
    TextureType_Vulkan = 2,
    TextureType_IOSurface = 3,
    TextureType_DirectX12 = 4,
    TextureType_DXGISharedHandle = 5,
    TextureType_Metal = 6
};

enum EColorSpace {
    ColorSpace_Auto = 0,
    ColorSpace_Gamma = 1,
    ColorSpace_Linear = 2
};

enum EVROverlayInputMethod {
    VROverlayInputMethod_None = 0,
    VROverlayInputMethod_Mouse = 1
};

// Overlay flags enum - commonly used overlay configuration flags
enum EVROverlayFlags {
    VROverlayFlags_NoDashboardTab = 8,
    VROverlayFlags_SendVRDiscreteScrollEvents = 64,
    VROverlayFlags_SendVRTouchpadEvents = 128,
    VROverlayFlags_ShowTouchPadScrollWheel = 256,
    VROverlayFlags_TransferOwnershipToInternalProcess = 512,
    VROverlayFlags_SideBySide_Parallel = 1024,
    VROverlayFlags_SideBySide_Crossed = 2048,
    VROverlayFlags_Panorama = 4096,
    VROverlayFlags_StereoPanorama = 8192,
    VROverlayFlags_SortWithNonSceneOverlays = 16384,
    VROverlayFlags_VisibleInDashboard = 32768,
    VROverlayFlags_MakeOverlaysInteractiveIfVisible = 65536,
    VROverlayFlags_SendVRSmoothScrollEvents = 131072,
    VROverlayFlags_ProtectedContent = 262144,
    VROverlayFlags_HideLaserIntersection = 524288,
    VROverlayFlags_WantsModalBehavior = 1048576,
    VROverlayFlags_IsPremultiplied = 2097152
};

// VR Controller State structure - basic controller input data
struct VRControllerState_t {
    uint32_t unPacketNum;
    uint64_t ulButtonPressed;
    uint64_t ulButtonTouched;
    float rAxis[5][2];
};

// Tracking result enum - needed before TrackedDevicePose_t
enum ETrackingResult {
    TrackingResult_Uninitialized = 1,
    TrackingResult_Calibrating_InProgress = 100,
    TrackingResult_Calibrating_OutOfRange = 101,
    TrackingResult_Running_OK = 200,
    TrackingResult_Running_OutOfRange = 201,
    TrackingResult_Fallback_RotationOnly = 300
};

// Matrix structures for tracking poses
struct HmdMatrix34_t {
    float m[3][4];
};

struct HmdVector3_t {
    float v[3];
};

struct TrackedDevicePose_t {
    HmdMatrix34_t mDeviceToAbsoluteTracking;
    HmdVector3_t vVelocity;
    HmdVector3_t vAngularVelocity;
    ETrackingResult eTrackingResult;
    bool bPoseIsValid;
    bool bDeviceIsConnected;
};

// Forward declarations and enums that are needed early

enum ETrackedPropertyError {
    TrackedProp_Success = 0,
    TrackedProp_WrongDataType = 1,
    TrackedProp_WrongDeviceClass = 2,
    TrackedProp_BufferTooSmall = 3,
    TrackedProp_UnknownProperty = 4,
    TrackedProp_InvalidDevice = 5,
    TrackedProp_CouldNotContactServer = 6,
    TrackedProp_ValueNotProvidedByDevice = 7,
    TrackedProp_StringExceedsMaximumLength = 8,
    TrackedProp_NotYetAvailable = 9,
    TrackedProp_PermissionDenied = 10,
    TrackedProp_InvalidOperation = 11,
    TrackedProp_CannotWriteToWildcards = 12,
    TrackedProp_IPCReadFailure = 13,
    TrackedProp_OutOfMemory = 14,
    TrackedProp_InvalidContainer = 15
};

// Basic structures

typedef TrackedDevicePose_t TrackedDevicePoses_t;

struct VREvent_Data_t {
    struct mouse_t {
        float x, y;
        uint32_t button;
    } mouse;
    // Add other event data as needed - we're keeping this minimal
};

struct VREvent_t {
    uint32_t eventType;
    TrackedDeviceIndex_t trackedDeviceIndex;
    float eventAgeSeconds;
    VREvent_Data_t data;
};

struct Texture_t {
    void* handle;
    ETextureType eType;
    EColorSpace eColorSpace;
};

// Minimal interface classes (stubs)
class IVRSystem {
public:
    virtual ETrackedDeviceClass GetTrackedDeviceClass(TrackedDeviceIndex_t unDeviceIndex) = 0;
    virtual bool IsTrackedDeviceConnected(TrackedDeviceIndex_t unDeviceIndex) = 0;
    virtual bool PollNextEvent(VREvent_t* pEvent, uint32_t uncbVREvent) = 0;
    virtual float GetFloatTrackedDeviceProperty(TrackedDeviceIndex_t unDeviceIndex, ETrackedDeviceProperty prop, ETrackedPropertyError* pError = nullptr) = 0;
    virtual uint32_t GetStringTrackedDeviceProperty(TrackedDeviceIndex_t unDeviceIndex, ETrackedDeviceProperty prop, char* pchValue, uint32_t unBufferSize, ETrackedPropertyError* pError = nullptr) = 0;
    virtual void GetDeviceToAbsoluteTrackingPose(ETrackingUniverseOrigin eOrigin, float fPredictedSecondsToPhotonsFromNow, TrackedDevicePose_t* pTrackedDevicePoseArray, uint32_t unTrackedDevicePoseArrayCount) = 0;
    virtual bool GetControllerStateWithPose(ETrackingUniverseOrigin eOrigin, TrackedDeviceIndex_t unControllerDeviceIndex, VRControllerState_t* pControllerState, uint32_t unControllerStateSize, TrackedDevicePose_t* pTrackedDevicePose) = 0;
    virtual ETrackedControllerRole GetControllerRoleForTrackedDeviceIndex(TrackedDeviceIndex_t unDeviceIndex) = 0;
};

class IVRCompositor {
public:
    virtual void GetLastPoses(TrackedDevicePoses_t* pRenderPoseArray, uint32_t unRenderPoseArrayCount, TrackedDevicePoses_t* pGamePoseArray, uint32_t unGamePoseArrayCount) = 0;
};

class IVROverlay {
public:
    virtual EVROverlayError CreateOverlay(const char* pchOverlayKey, const char* pchOverlayName, VROverlayHandle_t* pOverlayHandle) = 0;
    virtual EVROverlayError DestroyOverlay(VROverlayHandle_t ulOverlayHandle) = 0;
    virtual EVROverlayError SetOverlayWidthInMeters(VROverlayHandle_t ulOverlayHandle, float fWidthInMeters) = 0;
    virtual EVROverlayError SetOverlayAlpha(VROverlayHandle_t ulOverlayHandle, float fAlpha) = 0;
    virtual EVROverlayError SetOverlayInputMethod(VROverlayHandle_t ulOverlayHandle, EVROverlayInputMethod eInputMethod) = 0;
    virtual EVROverlayError SetOverlayTransformAbsolute(VROverlayHandle_t ulOverlayHandle, ETrackingUniverseOrigin eTrackingOrigin, const HmdMatrix34_t* pmatTrackingOriginToOverlayTransform) = 0;
    virtual EVROverlayError SetOverlayFromFile(VROverlayHandle_t ulOverlayHandle, const char* pchFilePath) = 0;
    virtual EVROverlayError SetOverlayIsDashboardOverlay(VROverlayHandle_t ulOverlayHandle, bool bIsDashboard) = 0;
    virtual EVROverlayError ShowOverlay(VROverlayHandle_t ulOverlayHandle) = 0;
    virtual EVROverlayError HideOverlay(VROverlayHandle_t ulOverlayHandle) = 0;
    virtual EVROverlayError SetOverlayTexture(VROverlayHandle_t ulOverlayHandle, const Texture_t* pTexture) = 0;
    virtual EVROverlayError SetOverlayFlag(VROverlayHandle_t ulOverlayHandle, EVROverlayFlags eOverlayFlag, bool bEnabled) = 0;
    virtual bool PollNextOverlayEvent(VROverlayHandle_t ulOverlayHandle, VREvent_t* pEvent, uint32_t uncbVREvent) = 0;
    virtual const char* GetOverlayErrorNameFromEnum(EVROverlayError error) = 0;
};

// Stub function implementations - these will fail gracefully
inline bool VR_IsRuntimeInstalled() { return false; }
inline bool VR_IsHmdPresent() { return false; }
inline IVRSystem* VR_Init(EVRInitError* peError, EVRApplicationType eType) {
    if (peError) *peError = VRInitError_Init_HmdNotFound;
    return nullptr;
}
inline void VR_Shutdown() {}
inline const char* VR_GetVRInitErrorAsEnglishDescription(EVRInitError eError) {
    return "OpenVR stub - VR runtime not available";
}
inline IVRCompositor* VRCompositor() { return nullptr; }
inline IVROverlay* VROverlay() { return nullptr; }
inline IVRSystem* VRSystem() { return nullptr; }

} // namespace vr