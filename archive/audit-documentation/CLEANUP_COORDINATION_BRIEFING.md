# CLEANUP COORDINATION
Directory Cleanup Specialist | Sept24,2025 | CRITICAL→IMMEDIATE | PM+Architect auth

## MISSION
Tech audit→critical drift→cleanup+simplify. Realign:recording app→audio routing. Role essential.

**Current:**97 docs vs 80 impl(1.2:1),453 files,recording throughout,over-eng,166 params
**Target:**5 docs max,<30 impl,3 modules(audio,vr,core),20 params,0 recording refs

## P1:RECORDING ELIMINATION

**Step1:Identify**
```bash
grep -r "Recording\|RecordingSession\|StartRecord\|StopRecord" src/
grep -r "TakeManager\|SessionManager\|FileOutput" src/
grep -r "recording\|session\|take" config/
```

**Step2:Remove(this week)**
HIGH:session mgmt code,file output in audio,recording UI,recording config,session/take mgmt,playback+file mgmt
COORD:notify Veteran before audio code,check Architect before core,confirm PM before structural,validate team before utils

**Step3:Verify**
After each:governance checks,verify deps,confirm build,test VR

## P2:DOCS REDUCTION(97→5)

**KEEP:**
1. README.md(overview+setup)→CREATE
2. CORRECTED_PROJECT_REQUIREMENTS.md→✅
3. DEVELOPMENT_GOVERNANCE_PROCEDURES.md→✅
4. API_DOCUMENTATION.md→CREATE
5. TROUBLESHOOTING_GUIDE.md→CREATE

**REMOVE IMMEDIATE(no coord):**beta/deploy/completion reports,Win build+valid,premium headset,Quest Pro+VR hw,security/authority,QA reports,cross-platform,agent authority,damage assess

**CAREFUL(check team):**arch analysis(may have info),tech audits(consolidate),VR troubleshoot(extract),build guides(consolidate to README)

**Process:**
1. Check refs before remove
2. Extract essentials to keep files
3. Announce removal
4. Track in git

## P3:MODULE SIMPLIFY

**Current excessive:**
modules/audio/(redundant),vr/(over-eng),ui/(complex),common/(bloat),core/(config complex),tests/(scattered),docs/(97),scripts/(complex),packaging/(premature),resources/(unused),examples/(not needed)

**Target(3 core):**
```
src/
├── core/(main.cpp,spatial_audio_engine.h/cpp,config.h/cpp,logger.h/cpp)
├── audio/(virtual_device.h/cpp,hrtf_processor.h/cpp,platform/)
├── vr/(vr_tracker.h/cpp,overlay_ui.h/cpp)
└── utils/(ring_buffer.h,math_utils.h)
```

**Phase1(wk1):Remove**
REMOVE:ui/(except overlay),packaging/,examples/,demo/,resources/,docs/(reduce to 5)
CONSOLIDATE:common/→core/utils/,performance/→core/(essential),network/→REMOVE,automation/→REMOVE

**Phase2(wk2):Simplify**
audio/:keep audio_engine+hrtf,remove recording+session,consolidate platform/,simplify config
vr/:keep vr_tracker,remove mic positioning+hw detect,simplify overlay,remove testing framework+caps mgr
core/:reduce config,remove recording config,simplify app struct,remove complex init

**Coord:**
Daily check-in:progress+blockers w/PM
Tech valid:Architect approval for major
Impl safety:Veteran review core changes
Git:all removals+consolidations committed

## P4:CONFIG SIMPLIFY(166→20)

**Step1:Audit**
```bash
grep -o '"[^"]*":' config/vr_binaural_config.json | sort | uniq -c
```

**Step2:Essential vs Excessive**
ESSENTIAL(~20):virtual device name+format,HRTF enable+dataset,VR tracking+overlay,perf(sample rate,buffer),logging+errors
EXCESSIVE(~146):recording params,automation,network,experimental(AI,GPU),UI custom,advanced perf(use defaults),debug/profiling,platform tweaks

**Step3:Process**
1. Backup→config_backup_166_params.json
2. Create clean 20-param version
3. Update code deps
4. Test w/defaults
5. Verify w/team

**Essential structure:**
```json
{
"audioRouting":{"virtualDeviceName":"VR Spatial Audio","sampleRate":48000,"bufferSize":128,"enableHRTF":true},
"spatialAudio":{"spatialMix":1.0,"distanceAttenuation":true,"maxDistance":10.0},
"vrIntegration":{"enableTracking":true,"overlayScale":0.3,"overlayPosition":[0,1.5,-0.8]},
"performance":{"threadPriority":"realtime","enableOptimizations":true,"targetLatency":10.0},
"logging":{"level":"info","enableConsole":true}
}
```

## COORDINATION

**Daily standup:**files removed today[n+types],modules consolidated[progress],config simplified[params reduced],blockers[issues],next plan[tasks],coord needs[team input]

**Daily checklist:**
- [ ] Check Veteran before core audio
- [ ] Confirm Architect before structural
- [ ] Notify PM significant removals
- [ ] Verify governance after phases
- [ ] Test after consolidations

**Weekly review:**docs reduction[removed/target],module simplify[consolidated/target],config cleanup[removed/target],recording elim[removed/total],build health[status],team coord[resolved/raised]

**Critical points:**
BEFORE:structural→Architect approval,core audio→Veteran coord,build→verify team,config→validate team,git→proper commits
DURING:progress→real-time updates,problems→immediate notify,testing→regular,docs→update tracking
AFTER:verify→no broken deps,perf→basic test,governance→auto checks,docs→update completion

## SUCCESS METRICS

**Daily:**files>20/day,config 10+params/day,build maintained,0 blocking issues

**Weekly:**
Wk1:docs 97→50(50%),config 166→100(40%),modules 6→4,recording total→50%
Wk2:docs 50→15,config 100→30,modules 4→3,recording 50%→90%
Wk3:docs 15→5(target),config 30→20(target),modules 3(optimized),recording 90%→100%

**Quality gates:**build functional,VR tracking+audio preserved,no team disruption,git tracked

## RISK/CONTINGENCY

**HIGH:Breaking core**
Mitigate:test after removal | Contingency:rollback via git | Coord:Veteran validation

**MEDIUM:Team disruption**
Mitigate:coord timing | Contingency:phased+team input | Coord:daily updates+resolution

**LOW:Config deps**
Mitigate:systematic analysis | Contingency:gradual removal+testing | Coord:team validation

**Emergency:**
If break:stop+notify,identify w/Veteran,rollback/fix,enhance testing
If coord fails:escalate PM,identify breakdown,emergency meeting,adjust procs

## COMPLETION VALIDATION

**Docs complete:**
- [ ] 5 files remain
- [ ] Excessive removed
- [ ] Essentials preserved
- [ ] Team access verified

**Modules simplified:**
- [ ] 3 core(core,audio,vr)
- [ ] Clear separation
- [ ] No redundancy
- [ ] Platform organized

**Config simplified:**
- [ ] 20 params max
- [ ] No recording
- [ ] Essential configurable
- [ ] Defaults work

**Recording eliminated:**
- [ ] 0 code refs
- [ ] No config
- [ ] No UI
- [ ] No file output in audio

**Final report:**
Quant:files removed,docs[from→to],config[from→to],modules[from→to],codebase size[from→to],recording eliminated
Qual:structure,team access,maintenance,focus,velocity
Coord:team collab,tech coord,process adherence,timeline
Recommendations:maintenance,future prevent,process improve,training

## CONCLUSION
Cleanup CRITICAL to realignment. Provides:priorities(recording elim+structural),coord protocols(team work),metrics(track+quality),risk mgmt(maintain function).

Client expects accountability,tracking,focus on correct reqs. Cleanup enables team deliver spatial audio routing they need.

Success=systematic approach+team coord+commitment to corrected vision.

PM+Architect+Veteran | Sept24,2025 | Start:Sept24 | Target:Oct15 | Daily coord required
