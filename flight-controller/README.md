# Setup

## Build Environment

### Windows

#### CLI

- download [arduino-cli](https://github.com/arduino/arduino-cli/releases)
- place the location of `arduino-cli.exe` in the `PATH` environment variable
- open a command prompt (`cmd.exe`)
- run `arduino-cli core update-index`
    - this is currently necessary on Windows to get the tool to properly obtain packages
- connect the arduino board to the system via USB
- run `arduino-cli board list` to view the board & identify its `Core`
- run `arduino-cli core install X` where `X` is the `Core` for the target board displayed in the previous step
- go to the `*/k10-drone/flight-controller` directory
- run `arduino-cli board attach -p X` where `X` is the COM port of the connected board, as displayed in the board list in the previous steps

#### Editor - VSCode

- create a local `settings.json` file in the `.vscode` directory
- populate the `settings.json` file with the following: 
```
{
    "C_Cpp.intelliSenseEngine": "Tag Parser",
    "C_Cpp.default.browse.path": 
        [ "C:/Users/USERNAME/AppData/Local/Arduino15/packages/**"
        , "${workspaceFolder}/flight-controller/submodules/i2cdevlib/Arduino/**"
    ]
}
```
  where `USERNAME` is your Windows user name

# Building

## Windows

- go to the `*/k10-drone/flight-controller` directory
- run `arduino-cli compile --libraries ./submodules/i2cdevlib/Arduino`
- run `arduino-cli upload` to upload the binaries to the device
