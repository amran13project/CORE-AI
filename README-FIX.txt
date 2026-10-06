CORE-AI GUI FIX

Replace/merge these files into C:\Users\HP\Desktop\Core AI.

Fixes:
1. Native executable resolves the project gui folder even when started from build\ or build\native\.
2. Native server supports /api/health and /api/chat used by the new GUI.
3. Existing /chat and status/capabilities endpoints remain supported.

After merging, rebuild CORE-AI with your usual Windows build command.
When launching, the console should print something like:
GUI root: C:\Users\HP\Desktop\Core AI\gui

Then open:
http://127.0.0.1:47821/

Do not delete package-native, package-test, source-snapshot, or web yet.
