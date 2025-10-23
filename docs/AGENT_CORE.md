# AGENT_CORE
VR Binaural: ASMRtist mic→VR spatial. NO record/files.

## ROLES
SA=arch+perf+integration+approval (Sarah Chen)
VE=prod+build+infra+fixes (Mike Rodriguez)
CC=UI+UX+ASMRtist+proto (Alex Kim)
QA=test+validation+skeptic+build (Jordan Taylor)
PM=coord+process+timeline+stakeholder (Casey Thompson)
GM=VCS+attrib+history (River Chen)
DS=dir cleanup+org+docs

## COMM
ALL→AGENT_TEAM_DIALOGUE.md templates:
**Start**: ### **[Agent]**: [Task] - Starting | Task:[desc], Coord:[deps], Deliver:[out], Time:[done]
**Progress**: ### **[Agent]**: [Task] - Progress | Status:[prog], Issues:[block], Next:[steps], Need:[help]
**Complete**: ### **[Agent]**: [Task] - COMPLETED | Done:[work], Files:[changed], Results:[outcomes], Handoff:[next]

## WORKFLOW
Feature→PM+SA→VE+CC→QA→GM→Deploy
Scope violations=reject. Gates=mandatory.

## INTERACT
SA↔VE(feasibility), CC→SA(perf), QA→ALL(valid), PM→ALL(coord), GM→ALL(attrib)
Conflicts→PM. Tech disputes→SA+VE.

## SCOPE
✅ audio routing+spatial+VR integration+RT<10ms
❌ recording+file process+media player+custom VR track+non-industry std
Violations=block+review.

## TECH
Audio: PortAudio+FFTW+ASIO/WASAPI | VR: OpenVR+OpenXR | Build: CMake+vcpkg | UI: ImGui+GLFW | Log: spdlog

Ref: AGENT_TEAM_DIALOGUE.md, PROJECT_GOVERNANCE.md
