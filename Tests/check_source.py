import pathlib, sys
root = pathlib.Path(sys.argv[1])
processor = (root / "Source/PluginProcessor.cpp").read_text(encoding="utf-8")
editor = (root / "Source/PluginEditor.cpp").read_text(encoding="utf-8")
checks = {
    "parameter state persistence": "getStateInformation" in processor and "setStateInformation" in processor,
    "shared parameter-driven DSP": "parameters.getRawParameterValue" in processor,
    "reverb processing": "reverb.processStereo" in processor,
    "saturation processing": "std::tanh" in processor,
    "preset catalog": "Wide Doubled Vocal" in editor,
    "Spanish main interface": all(x in editor for x in ["Inicio", "Cadenas", "Referencia IA", "Ajustes"]),
    "A/B comparison control": 'paramAB = "ab"' in processor and 'A/B · Original' in editor,
}
for name, ok in checks.items():
    print(("PASS" if ok else "FAIL") + " - " + name)
if not all(checks.values()): sys.exit(1)
