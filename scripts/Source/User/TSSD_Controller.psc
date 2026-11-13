Scriptname TSSD_Controller extends Quest

bool disabled = false
bool wakePending = false
int timerID = 7101

Event OnInit()
    RegisterForPlayerSleep()
    StartTimer(1.0, timerID)
EndEvent

; Waiting in a chair and cancelling the menu do not generate a completed sleep.
Event OnPlayerSleepStop(bool abInterrupted, ObjectReference akBed)
    if disabled || abInterrupted || wakePending
        return
    endif
    wakePending = true
    int token = TSSD_Native.Session()
    ; SleepStop can arrive before the SleepWaitMenu closes.
    while UI.IsMenuOpen("SleepWaitMenu") && token == TSSD_Native.Session()
        Utility.WaitMenuMode(0.1)
    endwhile
    ; One REAL second after close. This wait runs even if another menu is open.
    Utility.WaitMenuMode(1.0)
    if !disabled && token == TSSD_Native.Session()
        TSSD_Native.Wake(token)
    endif
    wakePending = false
EndEvent

Event OnTimer(int aiTimerID)
    if aiTimerID == timerID && !disabled
        TSSD_Native.Tick()
        StartTimer(1.0, timerID)
    endif
EndEvent

; Console: cqf TSSD_DreamController DisableDreams
; Wait a moment, save with the mod installed, THEN remove it.
Function DisableDreams()
    disabled = true
    UnregisterForPlayerSleep()
    CancelTimer(timerID)
    TSSD_Native.ClearDream()
EndFunction
