#include "PluginEditor.h"

namespace
{
void styleSlider (juce::Slider& s)
{
    s.setSliderStyle (juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag);
    s.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 64, 20);
    s.setColour (juce::Slider::rotarySliderFillColourId, juce::Colours::black);
    s.setColour (juce::Slider::thumbColourId, juce::Colours::black);
    s.setColour (juce::Slider::textBoxTextColourId, juce::Colours::black);
    s.setColour (juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
}
}

VocalAIStudioEditor::VocalAIStudioEditor (VocalAIStudioProcessor& p)
    : AudioProcessorEditor (&p), processor (p)
{
    setSize (760, 520); setResizable (true, true); setResizeLimits (620, 440, 1100, 760);
    title.setText ("VOCAL AI STUDIO", juce::dontSendNotification); title.setFont (juce::Font (22.0f, juce::Font::bold)); title.setColour (juce::Label::textColourId, juce::Colours::black); addAndMakeVisible (title);
    sectionBox.addItemList ({ "Inicio", "Cadenas", "Referencia IA", "Voz", "Ajustes" }, 1); sectionBox.setSelectedId (1); addAndMakeVisible (sectionBox);
    chainLabel.setText ("Cadena activa", juce::dontSendNotification); chainLabel.setFont (juce::Font (14.0f, juce::Font::bold)); addAndMakeVisible (chainLabel);
    presetBox.addItemList ({ "Clean Studio", "Modern Rap", "Melodic Trap", "Bright Urban", "Dark Vocal", "Atmospheric Vocal", "Hard-Tuned Vocal", "Natural Vocal", "Pop Lead", "Wide Doubled Vocal" }, 1); presetBox.onChange = [this] { processor.loadPreset (presetBox.getSelectedItemIndex()); }; addAndMakeVisible (presetBox);
    statusLabel.setText ("LISTO  ·  Procesamiento en tiempo real", juce::dontSendNotification); statusLabel.setColour (juce::Label::textColourId, juce::Colours::darkgrey); addAndMakeVisible (statusLabel);
    analyseButton.onClick = [this] { statusLabel.setText ("Análisis básico: ajusta el preset según tu escucha. Analizador IA pendiente.", juce::dontSendNotification); }; addAndMakeVisible (analyseButton);
    talkButton.onClick = [this] { statusLabel.setText ("Escribe una instrucción abajo. El control por lenguaje natural está pendiente.", juce::dontSendNotification); }; addAndMakeVisible (talkButton);
    compareButton.onClick = [this] { auto* v = processor.parameters.getParameter ("ab"); const bool next = processor.parameters.getRawParameterValue ("ab")->load() < 0.5f; v->setValueNotifyingHost (next ? 1.0f : 0.0f); compareButton.setButtonText (next ? "A/B · Original" : "A/B · Procesada"); }; addAndMakeVisible (compareButton);
    promptLabel.setText ("Instrucción", juce::dontSendNotification); addAndMakeVisible (promptLabel);
    prompt.setMultiLine (false); prompt.setTextToShowWhenEmpty ("Ej.: haz mi voz más brillante…", juce::Colours::grey); addAndMakeVisible (prompt);
    styleSlider (intensity); intensity.setRange (0.0, 1.0, 0.01); intensity.setValue (1.0); intensityLabel.setText ("Intensidad", juce::dontSendNotification); addAndMakeVisible (intensityLabel); addAndMakeVisible (intensity); mixAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (processor.parameters, "mix", intensity);
    styleSlider (bright); bright.setRange (-9.0, 9.0, 0.1); brightLabel.setText ("Brillo", juce::dontSendNotification); addAndMakeVisible (brightLabel); addAndMakeVisible (bright); brightAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (processor.parameters, "bright", bright);
    styleSlider (comp); comp.setRange (0.0, 1.0, 0.01); compLabel.setText ("Compresión", juce::dontSendNotification); addAndMakeVisible (compLabel); addAndMakeVisible (comp); compAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (processor.parameters, "comp", comp);
    styleSlider (reverb); reverb.setRange (0.0, 1.0, 0.01); reverbLabel.setText ("Reverb", juce::dontSendNotification); addAndMakeVisible (reverbLabel); addAndMakeVisible (reverb); reverbAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (processor.parameters, "reverb", reverb);
    advancedButton.onClick = [this] { statusLabel.setText ("Controles disponibles: filtro grave, entrada/salida y saturación. Más módulos en desarrollo.", juce::dontSendNotification); }; addAndMakeVisible (advancedButton);
}

void VocalAIStudioEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour (0xfff7f7f7));
    g.setColour (juce::Colours::white); g.fillRoundedRectangle (18.0f, 76.0f, static_cast<float> (getWidth() - 36), 112.0f, 8.0f);
    g.setColour (juce::Colour (0xffdddddd)); g.drawRoundedRectangle (18.0f, 76.0f, static_cast<float> (getWidth() - 36), 112.0f, 8.0f, 1.0f);
    g.setColour (juce::Colours::black); g.setFont (13.0f); g.drawText ("VOZ  /  CADENA", 34, 88, 160, 20, juce::Justification::left);
    g.setColour (juce::Colour (0xffe5e5e5)); g.fillRoundedRectangle (34.0f, 126.0f, static_cast<float> (getWidth() - 68), 40.0f, 4.0f);
    g.setColour (juce::Colour (0xff999999));
    for (int i = 0; i < getWidth() - 90; i += 9) { const float h = 5.0f + static_cast<float> ((i * 17) % 23); g.drawLine (46.0f + i, 146.0f - h * 0.5f, 46.0f + i, 146.0f + h * 0.5f, 1.2f); }
    g.setColour (juce::Colour (0xffdddddd)); g.drawLine (18.0f, 208.0f, static_cast<float> (getWidth() - 18), 208.0f, 1.0f);
}

void VocalAIStudioEditor::resized()
{
    auto area = getLocalBounds().reduced (20);
    title.setBounds (area.removeFromTop (34)); sectionBox.setBounds (getWidth() - 180, 24, 160, 28);
    area.removeFromTop (22); chainLabel.setBounds (area.removeFromTop (24)); presetBox.setBounds (area.removeFromTop (32).removeFromLeft (250));
    area.removeFromTop (142);
    auto row = area.removeFromTop (150); const int w = row.getWidth() / 4;
    auto place = [&row, w] (juce::Label& label, juce::Slider& slider) { auto cell = row.removeFromLeft (w); label.setBounds (cell.removeFromTop (22)); slider.setBounds (cell.reduced (4)); };
    place (intensityLabel, intensity); place (brightLabel, bright); place (compLabel, comp); place (reverbLabel, reverb);
    auto bottom = area.removeFromTop (34); analyseButton.setBounds (bottom.removeFromLeft (130)); bottom.removeFromLeft (8); talkButton.setBounds (bottom.removeFromLeft (64)); bottom.removeFromLeft (8); compareButton.setBounds (bottom.removeFromLeft (90)); bottom.removeFromLeft (8); advancedButton.setBounds (bottom.removeFromLeft (160));
    area.removeFromTop (6); promptLabel.setBounds (area.removeFromLeft (90)); prompt.setBounds (area);
    statusLabel.setBounds (20, getHeight() - 24, getWidth() - 40, 18);
}
