@rem prerequisites: install.bat has been run
@rem simply launch a command prompt with the proper environment variables
@rem necessary to build the project
set korlApplicationName=fc-telemetry-display
set korlApplicationVersion=r0
set korlGameDllFileName=korlDynApp
@rem To prevent simulation from becoming unstable at low frame rates due to lag
@rem	etc, `korlMinimumFrameRate` will ensure that the platform layer never
@rem	sends game code a deltaSeconds value less than 1/!
set korlMinimumFrameRate=30
call %windir%\system32\cmd.exe /k %korl_home%\misc\env.bat %~dp0
