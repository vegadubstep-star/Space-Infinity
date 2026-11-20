# ACTIVE — finish and release SPACE INFINITY 0.9.0

All requested source changes are implemented and compiled. Do not add more scope.

- 15 filters with live response graph + larger EDIT view and assignable vowel/morph.
- Original filterMode host range remains seven choices; appended characterFilterMode
  supplies 8 new models. The combined MAIN selector has 15. Old automation preserved.
- Stereo granular blending, loop crossfade (both directions), diffusion/warmth,
  Freeze and dedicated sampler panels with readable knobs/units.
- Actual sampler + granular octave fields replace inactive oscillator bindings.
- Voice Life (no amplitude modulation), A-to-B/B-to-A per-note oscillator journey.
- Larger performance/filter/envelope controls; new performance grid 3x2.
- 20 original Lush Trance leads in a separate import ZIP, with macros and tags.
- Seven quiet uplifting factory leads rebalanced (sustain, returns, macro gain),
  measured about 4–5 LU louder. Other 153 factory + Heritage200 recipes unchanged.
- Native version 0.9.0; 1223 parameters. All new feature amounts default bypass.

Final source build passed: /tmp/infinity-090-compatible-filter-build.log.
Compatible-filter focused checks 3/3 PASS (3.27s). Current exact-source full suite
running: /tmp/infinity-090-release-native-tests.log (27 tests).
Current exact-source audio verification (27 new/revised recipes) running:
/tmp/infinity-lead-final-compatible-verified.log, checkpoint
/workspace/work/lead-review090/Verified.json. Previous 27-recipe verification passed
before preserving the old filter host range; this final pass validates new encoding.
Renderer /tmp/RenderLush is rebuilt against final source. Inputs:
/workspace/work/AllLeads090-calibrated.json. Do not regenerate seeds/calibration.

FINAL Lush ZIP has been re-exported with characterFilterMode and packaged:
native-vst3/Presets/Lush20/SPACE_INFINITY_Lush_Trance_20.zip
SHA256 481ae6916cc8483f51167cff6950ba4163eb441718446426a8546500a2549932.
Native inventory exported and LF-normalized (1223). New bank native import is part
of the final suite. Python factory-edit importer test passes.
Actual MAIN/SAMPLER previews inspected at /workspace/work/preview090/audition;
final suite also captures actual GUI across sizes/DPI.

LOCAL VERIFICATION COMPLETE: all 27 native tests PASS (351.26s), Python importer
PASS, final native bank export/ZIP checks PASS. All 27 new/revised audio recipes
verified on the final renderer; max 4x true peak -0.349934 dBTP, max pre-safety peak
within .951 budget; 7 existing leads gain 3.84..5.05 LU at starting settings.
Evidence docs/audit/CHARACTER_090_NATIVE_TESTS.txt and LEADS_090_AUDIO_LEVELS.json.

Source/DSP commit 709d06c is pushed. Initial Windows run 44834594534 was
cancelled before publication to fix packaging allowlists and stale 0.7.2 README.
The new installer wrapper requires Lush20 + new reports; a content regression
check passes and rejects a missing bank. Synth/DSP source unchanged.

Packaging-only commit 0901f281d8555c2764ec76aeb6ea8638cdd9c722 is pushed.
Final Windows run: 44835072301; package job 136629273891.
https://github.com/vegadubstep-star/Space-Infinity/actions/runs/44835072301
Initial run 44834594534 is confirmed cancelled; publish was skipped.
Public ZIP verifier /tmp/verify-space-infinity090-release.py expects final 0901f28.

REMAINING: wait for this Windows workflow with
publish_release=true. Wait for Windows tests + actual installer verification,
then independently download/verify the public installer ZIP. Only then give user
the download link. No 0.9 release is published yet. Preserve owner untracked
source audition/ and __pycache__/ directories. Do not regenerate measured recipes.

Previous completed release follows for historical verification:

# COMPLETE — verified SPACE INFINITY 0.8.0 delivered/released

Final binary source: c70db12ca72e60daeebc484f81b764ba5b76cd5d, branchcodex/windows-build.
Release https://github.com/vegadubstep-star/Space-Infinity/releases/tag/v0.8.0
InstallerZIP https://github.com/vegadubstep-star/Space-Infinity/releases/download/v0.8.0/SPACE_INFINITY_0.8.0_Windows_x64_Installer.zip

ALL360actualnativeaudioaudits0issues,maximum4xtruepeak−0.295dBFS(target−0.3with
0.01measurementtolerance),32OPENtransitionsmaxdifference0.06LU, no finalnear-match
candidates.46extraheadroomfixtures: bassregisters, acidcutoffsweeps/heldcutoffs, IRstartup.
All24LinuxnativechecksPASS228.18s; focusedUI2/2PASS10.84safterWindowsfixturefix.
ALL24WINDOWSchecksPASS, PythonpresetZIPimportPASS, actualinstallerinstall/uninstall
PASS, installedbinaryequalmanualarchivePASS. SuccessfulCI44399691137 onexactc70source:
https://github.com/vegadubstep-star/Space-Infinity/actions/runs/44399691137
PreviousWindowsruns44391138236and44396210304 failed ONLY UItestfixtures; superseded.
NativeDPItestnowuses-supported1000x760min,keeps90%pixelmatch/allrenderer/scale checks;
keyboardtargetiscomputedafterpeerattachmentandplacedinsidevisibleuserArea.
OnlyproductionUIguardclampsnegativeLogoWidth; NOaudiodata/DSPchangesafteraudioaudit.

PublishedZIP independentlyDOWNLOADEDandVERIFIED:
ZIPSHA2564ca0e537ce3a3b67b7147c8c78f71cc6f217635637c572ebf2d7c06dfd324710
SetupSHA256b151484d8c5c4a63287781718eefdb755b12d00eb8b3dac1be5825a7fd5775a0
HeritageZIP43985d0daa8f43abd36aff0747b824931c62314c3cc1b73962ba69218b3035c0
CRC,setup/publichashes,x64PE,moduleinfo0.8.0,bankV2minimum0.8.0+200entries,
embedded360audioauditand1208parameterinventorychecked. Audioauditexactlymatchessource
afterWindowsCRLFnormalisation;SDKmoduleinfo'sstandardtrailingcommasparsedcorrectly.

Evidence docs/audit/TRACK_READY_080_{AUDIO_LEVELS,NATIVE_TESTS,WINDOWS_TESTS,
PUBLISHED_VERIFICATION,SIMILARITY,CONTROLS_TESTS,DPI_FIXTURE_TESTS}.*
Version0.8.0 has1208parameters,52waveforms,45sources(+6steadychoir/hybrids).
Factory160revised;Heritage200separateincludedZIP. Safeuneditedbankrefreshpreserves
locallyeditedsounds/metadata. CloseFL,ExtractAll,runSetup,IMPORTincludedHeritageZIP.
Olddownloadholds superseded;ownerexplicitlyaskedfordeliverywhendone. Noagentsauthorised.
Preserveowneruntracked audition/andcaches. No furthercode/audioverificationneeded
unlessownerrequestsnewchanges. Do notmistakeoldfailedlogsforfinalsuccessfulchecks.

Reproduction docs/research/TRACK_READY_080_REPRODUCTION.md;shippedJSONcurrenttruth.
Oldseedgeneratorswouldoverwritepolishedbank. Fullmeasurementcheckpoint remains
/workspace/work/track-ready080/review/Review.json;helper/tmp/RenderTrackReady fixed
0.8audioengineSHA. Temporaryexport44MB/cutoffWAV22MB/UIprobebinariesremovedafter
verification. Downloadedverifiedreleasefiles/workspace/work/release-verification/080.

Focused final filter/sampler/A-to-B/pitch/Lush bank tests: 4/4 PASS, 6.20s.
Earlier crash was stale ABI objects from a mid-build header edit; force-rebuilding
all PluginProcessor.h dependents resolved it. Keep headers fixed while building.

WINDOWS BLOCKER: run44835072301 failed SoundCharacter integration test with
SegFault after first4 tests passed. No release published. Cache successfully saved
(847MB), so subsequent native inputs can be reused. Added flushed diagnostic
checkpoints and Windows exception-code logging to SoundCharacterSmoke, and a
source-mtime restoration CI step to avoid rebuilding unchanged inputs. No DSP
changes. Diagnostic Linux integration still passes. Next dispatch diagnostic
Windows workflow with publish_release=false; read its failure checkpoint/code.
Artifact-upload quota is exhausted but optional; release uploads are unaffected.

LIFETIME ROOT CAUSE FIXED LOCALLY: octaveBindings had been declared before
octaveControls, so attachments were destroyed after their Slider targets. JUCE
SliderParameterAttachment::~ calls slider.removeListener, accessing destroyed
Slider::pimpl. Reordered fields and reset all octave bindings at start of editor
destructor. Added explicit sample/synth close/reopen coverage. Focused local
SoundCharacter/HardwareUI/PitchTravel/Modulation 4/4 PASS (14.18s). No DSP/preset
changes. Diagnostic Windows run44837546513 (publish=false) is still executing
older code to confirm checkpoint; final release run for this fix is next.

LATEST FIX COMMIT: 254f210a672651024e25670a37b50372b4ae5eff, pushed.
Final release run44838126358 / job136639521896 (publish_release=true).
Diagnostic old-code run44837546513 / job136637626935 has publish=false; leave
it to report the flushed checkpoint/Windows exception code. Both currently build.
Public ZIP verifier expects final254f210. No release published yet.

DIAGNOSTIC CONFIRMATION: old-code run44837546513 completed all sound checks
then crashed on teardown: Windows 0xc0000005 immediately after final success
line. This confirms the octave-binding destruction path. Logs job136637626935.
Final254f210 run44838126358 was cancelled before publish to also make filter/
sampler EDIT callouts owned by their creating components (no desktop orphan
borrowing processor). Expanded test opens each of 3 panels before editor teardown.
Focused4/4 PASS14.54s. New final commit/run is next; no DSP or preset changes.
