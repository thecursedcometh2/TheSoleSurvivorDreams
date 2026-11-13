# Validation status and in-game acceptance checks

Completed in this workspace: C++20 portable catalog/chance/expiry tests compiled
with warnings treated as errors and passed. Native Windows compilation, Papyrus
compilation, companion plugin construction, and game tests have not run.

Use a disposable profile/save, F4SE and Runtime Database matching the runtime,
and a companion quest built per BUILD.md. Check both F4SE and Papyrus logs.

1. Verify the native DLL loads and that `sqv TSSD_DreamController` reports the
   quest running. Test new and existing saves. If auto-start fails on an existing
   save, use `startquest TSSD_DreamController` diagnostically and fix packaging
   before release; do not treat that workaround as successful auto-start.
2. Cancel sleep and wait in a chair: neither should produce a dream. Complete
   sleep: at most one popup, never earlier than one second after menu close.
   Test interrupted sleep separately: this framework intentionally excludes it.
3. For a temporary debug build only, set chancePercent to 100 and narrow the
   selection to one catalog index at a time. Verify all 28 variants; then restore
   the 15% chance and full uniform catalog before release. Compare `player.getav`
   and maximum/current health/AP, not just a popup. Check the negative health
   effect with a nearly dead character and radiation-damaged maximum health.
4. Specifically verify `ExperienceMult` exists and modifies the same controlled
   XP reward by +/-10% from vanilla. Test environmental radiation resistance
   separately from food/drink radiation. These engine semantics are not covered
   by portable tests.
5. Expiry: advance just under 12 game hours, then past 12. Check only this mod's
   delta is removed. Test waiting, fast travel, and a sleep longer than 12 hours.
6. Trigger a second dream before expiry: previous delta must be removed before
   the new one is applied. A no-dream wake must retain an unexpired effect.
7. Save with a modifier active, quit, reload: no doubled bonus; original expiry
   retained. Load a different save and start a new game: no modifier or pending
   popup should leak across sessions. Save/load during the one-second wake delay.
8. Test repeated reloads, other mods modifying the same AV, and the disable
   console function. Do not uninstall until the delta is confirmed removed.

Do not distribute until Windows builds and game tests pass. If the pinned
CommonLib API or engine behavior differs, record and fix the actual failure;
portable tests are not a substitute for these checks.
