#include "PluginEditor.h"

AstrophiluxNebulaAudioProcessorEditor::AstrophiluxNebulaAudioProcessorEditor (AstrophiluxNebulaAudioProcessor& p)
 : AudioProcessorEditor(&p), processor(p)
{
    hero=juce::ImageFileFormat::loadFrom(BinaryData::Infinity_png,BinaryData::Infinity_pngSize);
    look.setColour(juce::Slider::rotarySliderFillColourId,juce::Colour(0xff6686ff));
    look.setColour(juce::Slider::rotarySliderOutlineColourId,juce::Colour(0xff202750));
    look.setColour(juce::Slider::thumbColourId,juce::Colour(0xffb9c6ff));
    for(size_t i=0;i<knobs.size();++i){
        auto& k=knobs[i]; k.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag); k.setTextBoxStyle(juce::Slider::TextBoxBelow,false,74,18); k.setLookAndFeel(&look); addAndMakeVisible(k);
        labels[i].setText(names[i],juce::dontSendNotification); labels[i].setJustificationType(juce::Justification::centred); labels[i].setColour(juce::Label::textColourId,juce::Colour(0xffdbe2ff)); addAndMakeVisible(labels[i]);
        attachments.push_back(std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(processor.state,ids[i],k));
    }
    setSize(900,540);
}

void AstrophiluxNebulaAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff050615));
    g.setGradientFill(juce::ColourGradient(juce::Colour(0xff11194a),0,0,juce::Colour(0xff07091d),900,540,false)); g.fillRoundedRectangle(getLocalBounds().toFloat().reduced(10),14);
    g.setColour(juce::Colour(0xff6686ff)); g.drawRoundedRectangle(getLocalBounds().toFloat().reduced(11),14,2);
    g.setFont(juce::FontOptions(29.f,juce::Font::bold)); g.drawText("ASTROPHILUX // NEBULA",30,22,520,42,juce::Justification::centredLeft);
    g.setColour(juce::Colour(0xff9aa8ef)); g.setFont(12.f); g.drawText("INFINITY SAMPLE INSTRUMENT  •  VST3 TEST BUILD",32,61,500,20,juce::Justification::centredLeft);
    auto imageArea=juce::Rectangle<float>(28,104,330,370); g.setColour(juce::Colours::white); g.fillRoundedRectangle(imageArea,12); g.setColour(juce::Colour(0xff7186ff)); g.drawRoundedRectangle(imageArea,12,2);
    if(hero.isValid()) g.drawImageWithin(hero,36,112,314,354,juce::RectanglePlacement::centred);
    g.setColour(juce::Colour(0xff6e86ff)); g.setFont(13.f); g.drawText("AMPLITUDE ENVELOPE",388,103,450,22,juce::Justification::centredLeft); g.drawText("ASTRO EFFECTS",388,298,450,22,juce::Justification::centredLeft);
    g.setColour(juce::Colour(0xff6975b3)); g.setFont(10.f); g.drawText("ROOT C3  //  16 VOICES  //  ORIGINAL ASTROPHILUX AUDIO",390,480,470,22,juce::Justification::centred);
}

void AstrophiluxNebulaAudioProcessorEditor::resized()
{
    const int xs[4]={390,505,620,735};
    for(int i=0;i<8;++i){ const int row=i/4, col=i%4, y=row?330:135; labels[(size_t)i].setBounds(xs[col],y,100,20); knobs[(size_t)i].setBounds(xs[col],y+18,100,115); }
}
