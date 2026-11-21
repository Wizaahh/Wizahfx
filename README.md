# Wizah FX – VST3 for FL Studio

Chain: Drive (tanh saturation) -> Tone (low-pass) -> Delay (feedback) -> Mix -> Output.

## Requirements
- CMake 3.22+
- Windows: Visual Studio 2022 (Desktop C++) | macOS: Xcode
- Git + internet (CMake downloads JUCE automatically)

## Build
    cmake -B build
    cmake --build build --config Release

The VST3 is built under build/WizahFX_artefacts/Release/VST3/.

## Easiest: automatic Windows installer
Push this folder to a GitHub repo -> Actions tab -> "Build Windows installer" ->
download the Wizah-FX-Setup artifact -> double-click the .exe.

## In FL Studio
Options > Manage plugins > Find installed plugins (verify scan), then add
"Wizah FX" from the plugin database under Installed > Effects.
