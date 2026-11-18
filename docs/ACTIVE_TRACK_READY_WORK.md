# Latest Windows correction — use the supported 1000px native minimum

Retry44396210304 source9a6cad8 passed23/24Windowschecks, includingPitchTravel.
Only HardwareUIfailed: nativeLogoSoftware1x2357/2430match;1.25x2415/2430match
(99.4%), but LOGICALwindowwidth was1000 afterWindowsenforcedresize minimum,
while fixtureasserted800. Logo rendering iscorrect; assertion's requestedsizewas
below interactive minimum. Fixture nowrequests/asserts1000x760 atall scales,
keeps90%pixelmatch and allengine/DPIchecks. Workflowadds--stop-on-failure so a
badUItrialdoesnotwaitthroughthewholelongaudioaudition. NoDSP/bank/productionUI
changes inthiscorrection. No0.8releasepublished. Needcommit/push[skipci]andnew
Windowsrunpublish_release=true, follow exactnewsourceSHA/run, verifydownloadthen
sendinstaller. Update/tmp/verify-space-infinity-release.py expectedSHA tonewcommit.
NativeLinuxcodeinside#Windowsnotchanged; previous24pass+focused2pass remainvalid.

---

# CURRENT Windows retry — monitor this exact run

Source9a6cad8c2277d9858a655d624ac66a6090f2af97 is saved/pushed.
NEW Windows workflow44396210304 is IN PROGRESS with publish_release=true:
https://github.com/vegadubstep-star/Space-Infinity/actions/runs/44396210304 .
Do not duplicate. Earlier44391138236failed UI fixtures only and publishednothing.
Native focused UI2/2passed afterfix;360audioaudit andbank unchanged.
/tmp/verify-space-infinity-release.py expectedsourceSHA already updatedto9a6cad8...
AfterWindowsallchecks/install/ZIPpublishpass, runthat independentdownload verifier,
saveactualWindows/test/installer verification evidence, then sendtheInstallerZIP.
Owner explicitly asks to receiveupdatewhendone; noapprovalquestionrequired.

---

# Latest Windows follow-up — first run failed UI fixtures only

Windowsrun44391138236 built/packagedsuccessfully, then22/24tests passed. All audio,
bank, effects and modulation passed. HardwareUIfailed at desktop200% (nativewindow
keptphysical800x760 => undersizedlogical400x380, corruptingfixturelayout); keyboard
hitpointwascomputedbeforepeerattachmentandcouldbeoccludedbytaskbar. Nothingpublished.
Actualartifact14468234085 screenshots inspected in/workspace/work/release-verification.

Fixes: Windowslogo test reapplies800x760 LOGICALsize at eachscale, keeps90%match,
assertslogicalsize, boundedmessagepump, RAII desktoprestore. Keyboardcomputeskeyafter
nativepeerattachmentandmovestargetkeyintodisplayuserArea; realnativehit/MIDI/audio
assertions retained. ProductionlogoWidthclampednonnegativeforundersizedhosts.
NO AUDIO DSP OR BANK SETTINGS CHANGED. NativeHardwareUI+PitchTravel2/2PASS10.84s,
docs/audit/TRACK_READY_080_DPI_FIXTURE_TESTS.txt. Fullnative24and360audioaudit remain
valid, previouslypassed. Needcommit/pushtheseUI-onlychanges[skipci], thennewWindows
workflowpublish_release=true. Cachefromfirstrunsaved. FollownewexactSHA/runID, then
independentlyverifyinstallerZIPanddeliver. /tmp/verify-space-infinity-release.py is
prepared, but updateexpectedsourcecommitfrom9c8ab...tonewSHA beforeusing it.
Firstfailedrunmustnotbererunwithoutpatchedsource; no0.8releaseexistedyet.

---

# Active release — SPACE INFINITY 0.8.0 (18 November 2026)

The owner explicitly asks to send the update/installer when finished. Earlier
holds are superseded. Do not ask for approval again. No subagents authorised.
Preserve the owner's untracked audition/ and script caches.

## Finished and verified

- Version0.8.0,1208parameters,52oscillatorfamilies (+15original band-limited/morphable
  additions),45sources (+6steady CC0 choir/synth hybrids). Legacy state transfer retained.
- Both factory160 + separate Heritage200 revised, simpler browser roles, original
  preset names, Makina palette, steady German-trance choirs and distinct voicings.
-32OPEN/LEADmacros lowerwetFX asfilter/sustain opens, with individual outputcompensation.
-Actual native audio reviewed across all360: base/repeated notes, chords, OPEN,
  all3assignablemacros raised; repeated convolution trials. Final audit0issues,
  highest4xtruepeak−0.295dBFS against−0.3target with0.01measurementtolerance,
  maximumOPEN/base difference0.06LU. No final spectral+envelope near-match candidates.
-46additional measured raw-headroom references cover4bassregisters, acid fullcutoff
  sweep+held160/8000Hz, and observedIR-startup maximum. Bass/acid stricter tests kept.
-ALL24 Linux native CTests PASS228.18s: /tmp/track-ready080-release-native-tests.log,
  copied docs/audit/TRACK_READY_080_NATIVE_TESTS.txt. Includes actualV2Heritage200
  import, update/edited-fileprotection/rollback, audio, UI, legacy/mode recall.
-Python factory-edits test passes. Six derivative/source asset SHA256 checks pass.
-Updated HeritageZIP nativeexported/recalled all1208params/routes, packagedV2 requiring
  version0.8.0. LatestSHA25643985d0daa8f43abd36aff0747b824931c62314c3cc1b73962ba69218b3035c0.
  A final StaccatoHall spiccato voicing was exported separately into fullcollection
  before repackaging; nativebankimporttestpassed afterward.
-Real1280×760 Main/FX native screenshots copied docs/images/hardware-main/fx.png.
  TemporaryUIprobe confirmed filterDecay knob=.350s,raw=.35,graph=D350ms (no UI bug).
-LFOactualShape/Custom names and yellowbadgepopup/pagevisibility fixed+tested.
-Release notes docs/TRACK_READY_080_UPDATE.md wired to Windows publisher/package,
  including updatedbank, flat-choirprovenance and compact actual-audio/native audits.

## Windows run now active — do not start a duplicate

Final source saved/pushed commit9c8ab994be1c8a8dccd830d7e5e57fe09007ed69.
Windows workflow44391138236 is IN PROGRESS, job135108796255, publish_release=true.
URLhttps://github.com/vegadubstep-star/Space-Infinity/actions/runs/44391138236 .
Setup/cache/Inno succeeded; currently Build and package. No0.8release existed before
this run. Follow its exact result, then independently verify release downloads.

## Remaining — proceed autonomously to delivery

1.Save/push finalsource changes with[skip ci] (prior pushed905f71c is earlier draft).
2.Dispatch .github/workflows/space-infinity-windows.yml oncodex/windows-build.
  All native/audio checks are finished, so publish_release=true is authorised;
  workflow itself publishes only after Windows24checks, installerinstall/uninstall,
  VST3binaryequality and ZIPchecksumverification. GitHubconnector lacks dispatch;
  authenticatedghCLI/git available. Do not rerun expensive native/audioaudits unless
  new changes/failures justify it.
3.Monitor exactworkflow/sourceSHA. Use connected GitHub fetch_workflow_run_jobs tool
  for supported read operations (repo_full_name/run_id), CLI for dispatch/run discovery.
4.After successful publication independently download0.8InstallerZIP and checksum,
  verify archive CRC, embeddedSetup hash againstINSTALLER_SHA256 and releaseSHA256SUMS,
  includedHeritageZIP hash matches source43985..., and main VST3/installer checks.
5.Send verified installerZIP release link plus brief closeFL/extract/setup/import-bank
  instructions. No final download has been sent yet, and no0.8Windowsbuild run yet.

## Reproduction and files

The shipped JSON recipes are current authoring truth. Historical seed generators
would overwrite later work; see docs/research/TRACK_READY_080_REPRODUCTION.md.
Research actualreadsources recorded docs/research/TRACK_READY_080_RESEARCH.md.

scripts/review-track-ready.py uses2native childprocesses (CPUquota2), atomic
checkpoints and deletes only generatedWAVs/jobfolders. Reuses validvariants; checks
binarySHA+canonicalrowSHA. Adds max_ir1/2 for convolution. Handles individualerrors
without losing later completed jobs. Nativehelper /tmp/RenderTrackReady fixedSHA
matches final0.8DSP/samples; only V2importheader/tests changed afterward, no audioDSP.
MainnativeSmoke rebuilt against latest factoryJSON; generatedresource bytes checked.

scripts/calibrate-track-ready.py defaults --mode peak, not fixed−24LUFS. --refine
changes only actualexcess/unusedgain/OPENdifference, with0.05dB correctionmargin.
External refs in scripts/track-ready-headroom-references.json use recipehash guards
and measured pre-master peaks. scripts/verify-track-ready.py checks all360 actual
currentrow/binaryhashes, truepeak/rawceilings,unusedgain andOPENtransitions.

Finalmeasuredprofile /workspace/work/track-ready080/review/Review.json.
Earlierprofiles/calibration records stay outside repo in/workspace/work/track-ready080.
Compact audits checkedin docs/audit/TRACK_READY_080_*. Initial failed attempts are
superseded by final0issues/24pass records; do not confuse earlier logs with final.

Nativebuild native-vst3/build-linux. CMake/CTest under
/workspace/work/build-tools/cmake/data/bin. GUI DISPLAY=:99 and
XDG_CACHE_HOME=/workspace/work/runtime-cache. No backgroundaudio/nativejobs remain.
OptionalLinuxStandalone/VST3wrapper build was stopped (100LTOjobs), not needed for
Windowsdelivery; do not claim finalLinuxwrapperpackage. Windows builds fullformats.
Our temporary Exported(44MB), Cutoff WAVs(22MB) and UIprobe binaries(70MB) removed
AFTER package/native verification; preserve reviewJSON+cutoff raw-profileJSON.
Disk~293MBfree, enough for installerZIP. Never delete owner audition/caches wholesale.

GitHub source vegadubstep-star/Space-Infinity, branchcodex/windows-build.
Exactenvironment IDccarenv_b64_Y2NhcmVudl82OTBhZmNlYzA3YzQ4MTkxOTBiZjA2ZWI5OTU3YjI3Ng;
if starting, wait usingexact ID; preserve work across environment restarts.
