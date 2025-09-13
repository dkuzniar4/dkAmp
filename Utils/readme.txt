1. Generate test tone with:
py testToneGen.py

2. Play test tone through guitar amp or overdrive fx and record it. Cut recorded file to the same length, with corresponding test tones. Export to processed.wav file etc.

3. Run
py transitionGen.py .\test_tones.wav .\processed.wav ampProfile.h to generate ampProfile.h with transistion table: float tran[3][AMP_STEPS].

4. Copy tran table and rename it and paste into ampProfiles.h file.