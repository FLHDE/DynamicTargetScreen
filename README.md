# Dynamic Target Screen
Reimplementation of the [Dynamic Target Screen plugin](https://www.moddb.com/mods/dynamic-target-screen) by [Venemon](https://www.moddb.com/members/venemon).
This version is compatible with the Advanced Widescreen HUD plugin by ensuring the Switch To Target button remains visible.
If you use the Advanced Widescreen HUD, make sure to include `TargetScreen.dll` *after* `HudTarget.dll` in `dacom.ini`'s `[Libraries]` section.

To enable the top-down view by default, apply the following hex edit: `Freelancer.exe, 214470, 00 -> 01`.
If you would like to update the ids_name of the Switch To Target button tooltip: `Freelancer.exe, E15D3, 1535i`. Moreover, the IDS of the Switch To Target key and rollover help can be found by locating the `USER_SWITCH_TO_TARGET` `[KeyCmd]` entry in `DATA\INTERFACE\keymap.ini`.
