#include "PluginEditor.h"
#include <cmath>

static const auto bg=juce::Colour(0xff03151b), panel=juce::Colour(0xff06252c), cyan=juce::Colour(0xff64fff0), blue=juce::Colour(0xff55a8ff);

void NebulaLookAndFeel::drawRotarySlider(juce::Graphics& g,int x,int y,int w,int h,float pos,float start,float end,juce::Slider&)
{
    auto r=juce::Rectangle<float>((float)x,(float)y,(float)w,(float)h).reduced(8);
    const float radius=juce::jmin(r.getWidth(),r.getHeight())*.5f; const juce::Point<float> c=r.getCentre();
    g.setColour(juce::Colour(0xff020b10)); g.fillEllipse(r);
    g.setColour(juce::Colour(0xff174b58)); g.drawEllipse(r,2);
    juce::Path arc; arc.addCentredArc(c.x,c.y,radius-2,radius-2,0,start,start+pos*(end-start),true);
    g.setColour(cyan); g.strokePath(arc,juce::PathStrokeType(3.0f));
    auto a=start+pos*(end-start); juce::Path tick; tick.startNewSubPath(c); tick.lineTo(c+juce::Point<float>(std::sin(a),-std::cos(a))*(radius-9));
    g.setColour(juce::Colours::white); g.strokePath(tick,juce::PathStrokeType(2));
}

AstrophiluxNebulaAudioProcessorEditor::AstrophiluxNebulaAudioProcessorEditor(AstrophiluxNebulaAudioProcessor& p):AudioProcessorEditor(&p),processor(p)
{
    hero=juce::ImageFileFormat::loadFrom(BinaryData::Infinity_png,BinaryData::Infinity_pngSize);
    for(size_t i=0;i<knobs.size();++i){
        knobs[i].setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag); knobs[i].setTextBoxStyle(juce::Slider::NoTextBox,false,0,0); knobs[i].setLookAndFeel(&look); addAndMakeVisible(knobs[i]);
        labels[i].setText(names[i],juce::dontSendNotification); labels[i].setJustificationType(juce::Justification::centred); labels[i].setColour(juce::Label::textColourId,juce::Colour(0xffd9fbff)); labels[i].setFont(11.f); addAndMakeVisible(labels[i]);
        attachments.push_back(std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(processor.state,ids[i],knobs[i]));
    }
    setResizable(true,true); setResizeLimits(1000,650,1600,1000); setSize(1400,860);
}
AstrophiluxNebulaAudioProcessorEditor::~AstrophiluxNebulaAudioProcessorEditor(){ for(auto& k:knobs) k.setLookAndFeel(nullptr); }

void AstrophiluxNebulaAudioProcessorEditor::drawPanel(juce::Graphics& g,juce::Rectangle<float> r,const juce::String& title,juce::Colour accent){
    g.setColour(panel.withAlpha(.94f)); g.fillRoundedRectangle(r,8); g.setColour(juce::Colour(0xff17606b)); g.drawRoundedRectangle(r,8,1);
    g.setColour(accent); g.setFont(juce::FontOptions(13.f,juce::Font::bold)); g.drawText(title,r.toNearestInt().reduced(14).removeFromTop(24),juce::Justification::centredLeft);
}
void AstrophiluxNebulaAudioProcessorEditor::drawWave(juce::Graphics& g,juce::Rectangle<float> r,juce::Colour c,float phase){
    g.setColour(juce::Colour(0xff0b3a43)); for(int i=1;i<8;i++) g.drawVerticalLine((int)(r.getX()+r.getWidth()*i/8),r.getY(),r.getBottom());
    juce::Path p; for(int x=0;x<(int)r.getWidth();++x){ float t=(float)x/r.getWidth()*juce::MathConstants<float>::twoPi*2.2f; float yy=r.getCentreY()+std::sin(t+phase)*r.getHeight()*.28f*(.7f+.3f*std::sin(t*.35f)); if(x==0)p.startNewSubPath(r.getX()+x,yy);else p.lineTo(r.getX()+x,yy); }
    g.setColour(c.withAlpha(.25f)); for(int i=-3;i<=3;i++){ auto q=p; q.applyTransform(juce::AffineTransform::translation(0,(float)i*3)); g.strokePath(q,juce::PathStrokeType(1)); } g.setColour(c); g.strokePath(p,juce::PathStrokeType(2));
}
void AstrophiluxNebulaAudioProcessorEditor::drawKeyboard(juce::Graphics& g,juce::Rectangle<int> r){
    g.setColour(juce::Colour(0xff031319)); g.fillRoundedRectangle(r.toFloat(),7); auto keys=r.reduced(120,12); int whites=42; float kw=(float)keys.getWidth()/whites;
    for(int i=0;i<whites;i++){ auto k=juce::Rectangle<float>(keys.getX()+i*kw,(float)keys.getY(),kw-1,(float)keys.getHeight()); g.setColour(juce::Colour(0xffeaf6f6)); g.fillRect(k); g.setColour(juce::Colour(0xff497078)); g.drawRect(k); }
    const int pat[5]={1,3,6,8,10}; for(int oct=0;oct<6;oct++) for(int j:pat){ int wi=oct*7+(j<5?j/2:j/2); float x=keys.getX()+(wi+0.68f)*kw; g.setColour(juce::Colour(0xff07191f)); g.fillRoundedRectangle(x,keys.getY(),kw*.62f,keys.getHeight()*.62f,2); }
    g.setColour(cyan); g.setFont(10.f); g.drawText("PITCH     MOD",r.getX()+18,r.getBottom()-25,90,18,juce::Justification::centred); g.drawText("OCT  0",r.getRight()-90,r.getBottom()-25,70,18,juce::Justification::centred);
}
void AstrophiluxNebulaAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(bg); auto b=getLocalBounds(); int W=b.getWidth(),H=b.getHeight();
    if(hero.isValid()){ g.setOpacity(.48f); g.drawImage(hero,juce::Rectangle<float>(0,0,(float)W,125),juce::RectanglePlacement::fillDestination); g.setOpacity(1); }
    g.setColour(juce::Colour(0xcc00151b)); g.fillRect(0,0,W,125);
    g.setColour(cyan); g.setFont(juce::FontOptions(13.f,juce::Font::bold)); g.drawText("A S T R O P H I L U X",28,18,330,22,juce::Justification::centredLeft);
    g.setFont(juce::FontOptions(39.f,juce::Font::plain)); g.drawText("N E B U L A",28,39,380,50,juce::Justification::centredLeft);
    g.setColour(juce::Colour(0xffb5e9ee)); g.setFont(10.f); g.drawText("INSTRUMENTS FOR FURTHER PLACES",30,91,360,18,juce::Justification::centredLeft);
    auto preset=juce::Rectangle<float>(W*.38f,28,W*.25f,52); g.setColour(juce::Colour(0xdd03191f)); g.fillRoundedRectangle(preset,8); g.setColour(juce::Colour(0xff21717c));g.drawRoundedRectangle(preset,8,1);g.setColour(juce::Colours::white);g.setFont(20.f);g.drawText("‹          Synth Pad 3          ›",preset.toNearestInt(),juce::Justification::centred);
    g.setColour(cyan);g.setFont(11.f);g.drawText("Synth Pads  ›  Synth Pad 3",(int)preset.getX(),82,(int)preset.getWidth(),18,juce::Justification::centred);
    g.setColour(juce::Colours::white);g.setFont(12.f);g.drawText("MASTER VOLUME",W-205,70,160,18,juce::Justification::centred);
    g.setColour(juce::Colour(0xff061e25));g.fillEllipse((float)W-150,18,55,55);g.setColour(cyan);g.drawEllipse((float)W-150,18,55,55,2);
    int side=220, top=130, bottom=145, gap=10; auto sideR=juce::Rectangle<float>(10,(float)top,(float)side,H-top-bottom-10); drawPanel(g,sideR,"PRESETS",cyan);
    g.setColour(juce::Colour(0xff0c3038));g.fillRoundedRectangle(22,180,196,35,5);g.setColour(juce::Colour(0xffa7cbd0));g.setFont(12.f);g.drawText("⌕  Search presets...",32,184,175,27,juce::Justification::centredLeft);
    const char* cats[]={"★  Favorites","▣  Synth Pads","    Synth Pad 3","    Infinite Haze","    Soft Orbit","▣  Keys","▣  Plucks","▣  Bass","▣  Textures","▣  Ambient","▣  User"};
    for(int i=0;i<11;i++){ int y=230+i*36; if(i==2){g.setColour(juce::Colour(0xff08758a));g.fillRect(20,y-4,200,32);} g.setColour(i==2?juce::Colours::white:juce::Colour(0xffd1edf0));g.drawText(cats[i],32,y,180,24,juce::Justification::centredLeft); }
    auto main=juce::Rectangle<float>((float)side+20,(float)top,(float)W-side-30,(float)H-top-bottom-10);
    g.setColour(juce::Colour(0xffbfeff0));g.setFont(13.f);g.drawText("SYNTH        MODULATION        EFFECTS        ARP & SEQ        GLOBAL",(int)main.getX()+15,top,(int)main.getWidth()-30,34,juce::Justification::centredLeft);g.setColour(cyan);g.fillRect(juce::Rectangle<float>(main.getX()+10.0f,(float)top+32.0f,130.0f,2.0f));
    float y=top+45, half=(main.getWidth()-10)/2; auto oscA=juce::Rectangle<float>(main.getX(),y,half,205),oscB=juce::Rectangle<float>(main.getX()+half+10,y,half,205); drawPanel(g,oscA,"⏻  OSCILLATOR A",cyan);drawPanel(g,oscB,"⏻  OSCILLATOR B",blue);
    drawWave(g,oscA.reduced(15).withTrimmedTop(30).withTrimmedBottom(65),cyan,0);drawWave(g,oscB.reduced(15).withTrimmedTop(30).withTrimmedBottom(65),blue,1.1f);
    auto env=juce::Rectangle<float>(main.getX(),y+215,main.getWidth()*.61f,190), fil=juce::Rectangle<float>(env.getRight()+10,y+215,main.getRight()-env.getRight()-10,190);drawPanel(g,env,"AMP ENVELOPE",cyan);drawPanel(g,fil,"⏻  FILTER  •  LOWPASS 24",blue);
    auto graph=env.reduced(18).withTrimmedTop(30).withTrimmedBottom(68);juce::Path ep;ep.startNewSubPath(graph.getX(),graph.getBottom());ep.lineTo(graph.getX()+graph.getWidth()*.22f,graph.getY()+8);ep.lineTo(graph.getX()+graph.getWidth()*.48f,graph.getCentreY());ep.lineTo(graph.getX()+graph.getWidth()*.72f,graph.getCentreY());ep.lineTo(graph.getRight(),graph.getBottom());g.setColour(cyan);g.strokePath(ep,juce::PathStrokeType(2));
    auto fg=fil.reduced(18).withTrimmedTop(34).withTrimmedBottom(70);juce::Path fp;fp.startNewSubPath(fg.getX(),fg.getCentreY()-20);for(int x=0;x<(int)fg.getWidth();x++){float t=(float)x/fg.getWidth();fp.lineTo(fg.getX()+x,fg.getCentreY()-20+55.f*std::pow(t,4.f));}g.setColour(blue);g.strokePath(fp,juce::PathStrokeType(2));
    auto fxY=y+415; float fw=(main.getWidth()-20)/3;drawPanel(g,{main.getX(),fxY,fw,105},"⏻  DRIFT  •  SUBTLE MOVEMENT",cyan);drawPanel(g,{main.getX()+fw+10,fxY,fw,105},"⏻  ECHO  •  AMBIENT DELAY",blue);drawPanel(g,{main.getX()+2*(fw+10),fxY,fw,105},"⏻  SPACE  •  LUSH REVERB",juce::Colour(0xff8b83ff));
    drawKeyboard(g,{10,H-bottom+10,W-20,bottom-20});
    g.setColour(juce::Colour(0xff58cbd5));g.setFont(9.f);g.drawText("ASTROPHILUX NEBULA  •  SYNTH PAD 3",18,H-16,300,12,juce::Justification::centredLeft);
}
void AstrophiluxNebulaAudioProcessorEditor::resized()
{
    int W=getWidth(),H=getHeight(),side=220,top=130,bottom=145; float mx=side+20,my=top+45,mw=W-side-30,half=(mw-10)/2;
    auto place=[&](int i,float x,float y,float w=78.f,float h=78.f){labels[(size_t)i].setBounds((int)x,(int)y-16,(int)w,16);knobs[(size_t)i].setBounds((int)x,(int)y,(int)w,(int)h);};
    float envY=my+215, envW=mw*.61f; for(int i=0;i<4;i++) place(i,side+38+i*(envW-95)/3,envY+108);
    place(4,side+envW+85,envY+108);
    float fxY=my+415,fw=(mw-20)/3;place(5,side+fw*.5f-18,fxY+30,72,72);place(6,side+fw+10+fw*.5f-18,fxY+30,72,72);place(7,side+2*(fw+10)+fw*.5f-18,fxY+30,72,72);
}