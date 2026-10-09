// VIMEK native panel. GPL-3.0.
#import "VimekPanelController.h"
#import "AppDelegate.h"
#include "../../engine/InputMethods.h"
extern AppDelegate *appDelegate;
extern int vLanguage, vInputType, vCheckSpelling, vUseSmartSwitchKey, vSwitchKeyStatus, vCodeTable;
extern void RequestNewSession(void);
extern void OnSpellCheckingChanged(void);

@interface VimekSurface : NSView
@end
@implementation VimekSurface
-(BOOL)isFlipped { return YES; }
-(void)viewDidChangeEffectiveAppearance {[super viewDidChangeEffectiveAppearance];[self setNeedsDisplay:YES];}
-(void)drawRect:(NSRect)dirtyRect {
    [NSColor.windowBackgroundColor setFill];NSRectFill(self.bounds);
    [NSColor.separatorColor setStroke];
    for(NSNumber *y in @[@254,@310,@419,@484]) {
        NSBezierPath *line=[NSBezierPath bezierPath];[line moveToPoint:NSMakePoint(32,y.doubleValue)];
        [line lineToPoint:NSMakePoint(628,y.doubleValue)];[line stroke];
    }
    CGFloat v[][2]={{14,13},{23,13},{32,40},{41,13},{50,13},{36,51},{28,51}};
    CGFloat e[][2]={{17,13},{48,13},{48,20},{25,20},{25,29},{45,29},{45,36},{25,36},{25,44},{48,44},{48,51},{17,51}};
    NSBezierPath *path=[NSBezierPath bezierPath];
    for(int i=0;i<(vLanguage?7:12);++i){CGFloat x=vLanguage?v[i][0]:e[i][0],y=vLanguage?v[i][1]:e[i][1];
        NSPoint p=NSMakePoint(588+x*.8,9.4+y*.8);if(!i)[path moveToPoint:p];else[path lineToPoint:p];}
    [path closePath];path.lineWidth=1.2;[NSColor.labelColor setStroke];[path stroke];
}
@end

@implementation VimekPanelController {
    NSSegmentedControl *_language, *_method;
    NSTextField *_example, *_footer;
    NSButton *_spell, *_smart, *_hotkey;
}
-(NSTextField*)label:(NSString*)value frame:(NSRect)frame size:(CGFloat)size muted:(BOOL)muted {
    NSTextField *label=[NSTextField labelWithString:value];label.frame=frame;
    label.font=[NSFont systemFontOfSize:size weight:size>25?NSFontWeightSemibold:NSFontWeightRegular];
    label.textColor=muted?NSColor.secondaryLabelColor:NSColor.labelColor;
    [self.window.contentView addSubview:label];return label;
}
-(NSButton*)button:(NSString*)title frame:(NSRect)frame action:(SEL)action {
    NSButton *button=[NSButton buttonWithTitle:title target:self action:action];button.frame=frame;
    button.bezelStyle=NSBezelStyleRounded;button.font=[NSFont systemFontOfSize:14];
    [self.window.contentView addSubview:button];return button;
}
-(instancetype)init {
    NSWindow *window=[[NSWindow alloc] initWithContentRect:NSMakeRect(0,0,660,532)
        styleMask:NSWindowStyleMaskTitled|NSWindowStyleMaskClosable|NSWindowStyleMaskMiniaturizable backing:NSBackingStoreBuffered defer:NO];
    self=[super initWithWindow:window];if(!self)return nil;
    window.title=@"VIMEK";window.releasedWhenClosed=NO;window.delegate=self;
    // No explicit appearance: native semantic colors follow macOS live.
    window.contentView=[[VimekSurface alloc] initWithFrame:NSMakeRect(0,0,660,532)];
    [self label:@"V I M E K" frame:NSMakeRect(32,23,500,24) size:13 muted:NO];
    _language=[NSSegmentedControl segmentedControlWithLabels:@[@"Tiếng Việt",@"English"] trackingMode:NSSegmentSwitchTrackingSelectOne target:self action:@selector(languageChanged:)];
    _language.frame=NSMakeRect(32,78,596,36);_language.segmentStyle=NSSegmentStyleRounded;
    for(int i=0;i<2;i++)[_language setWidth:292 forSegment:i];[window.contentView addSubview:_language];
    [self label:@"KIỂU GÕ" frame:NSMakeRect(32,134,596,24) size:12 muted:YES];
    NSMutableArray *labels=[NSMutableArray array];for(int i=0;i<VIMEK_INPUT_METHOD_COUNT;i++)[labels addObject:[NSString stringWithUTF8String:vimekInputMethodName(i)]];
    _method=[NSSegmentedControl segmentedControlWithLabels:labels trackingMode:NSSegmentSwitchTrackingSelectOne target:self action:@selector(methodChanged:)];
    _method.frame=NSMakeRect(32,166,596,36);for(int i=0;i<4;i++)[_method setWidth:145 forSegment:i];[window.contentView addSubview:_method];
    _example=[self label:@"" frame:NSMakeRect(32,215,596,26) size:14 muted:YES];
    [self label:@"Phím chuyển Việt / Anh" frame:NSMakeRect(32,273,360,25) size:15 muted:NO];
    _hotkey=[self button:@"⌃ Control + ⌥ Option" frame:NSMakeRect(412,262,216,40) action:@selector(resetShortcut:)];
    _hotkey.toolTip=@"Bấm để dùng Control + Option. Đặt phím khác trong Nâng cao.";
    [self label:@"Kiểm tra chính tả" frame:NSMakeRect(32,332,410,26) size:15 muted:NO];
    _spell=[self button:@"Bật" frame:NSMakeRect(538,317,90,40) action:@selector(spellingChanged:)];
    [self label:@"Nhớ chế độ theo ứng dụng" frame:NSMakeRect(32,381,410,26) size:15 muted:NO];
    _smart=[self button:@"Bật" frame:NSMakeRect(538,366,90,40) action:@selector(smartChanged:)];
    [self button:@"Gõ tắt" frame:NSMakeRect(32,432,188,40) action:@selector(macros:)];
    [self button:@"Chuyển mã" frame:NSMakeRect(232,432,188,40) action:@selector(convert:)];
    [self button:@"Nâng cao" frame:NSMakeRect(432,432,196,40) action:@selector(advanced:)];
    _footer=[self label:@"" frame:NSMakeRect(32,499,596,22) size:12 muted:YES];
    [self refresh];[window center];return self;
}
-(void)refresh {
    _language.selectedSegment=vLanguage?0:1;_method.selectedSegment=vimekNormalizeInputMethod(vInputType);
    _example.stringValue=vInputType==1?@"Ví dụ: tie6ng1 Vie6t5 → tiếng Việt":@"Ví dụ: tieengs Vieetj → tiếng Việt";
    _spell.title=vCheckSpelling?@"Bật":@"Tắt";_smart.title=vUseSmartSwitchKey?@"Bật":@"Tắt";
    _spell.accessibilityLabel=vCheckSpelling?@"Kiểm tra chính tả: Bật":@"Kiểm tra chính tả: Tắt";
    _smart.accessibilityLabel=vUseSmartSwitchKey?@"Nhớ chế độ theo ứng dụng: Bật":@"Nhớ chế độ theo ứng dụng: Tắt";
    NSMutableArray *keys=[NSMutableArray array];
    if(vSwitchKeyStatus&0x100)[keys addObject:@"⌃ Control"];
    if(vSwitchKeyStatus&0x200)[keys addObject:@"⌥ Option"];
    if(vSwitchKeyStatus&0x400)[keys addObject:@"⌘ Command"];
    if(vSwitchKeyStatus&0x800)[keys addObject:@"⇧ Shift"];
    unsigned character=((unsigned)vSwitchKeyStatus>>24)&255;
    if(character!=0xFE&&character)[keys addObject:[NSString stringWithFormat:@"%C",(unichar)character]];
    _hotkey.title=keys.count?[keys componentsJoinedByString:@" + "]:@"Chưa đặt";
    NSArray *codes=@[@"Unicode",@"TCVN3",@"VNI Windows",@"Unicode tổ hợp",@"CP1258"];
    _footer.stringValue=codes[(vCodeTable>=0&&vCodeTable<5)?vCodeTable:0];
    [self.window.contentView setNeedsDisplay:YES];
}
-(void)windowDidBecomeKey:(NSNotification*)notification {[self refresh];}
-(void)languageChanged:(NSSegmentedControl*)sender {
    if(vLanguage!=(sender.selectedSegment==0)){[appDelegate onImputMethodChanged:YES];RequestNewSession();}[self refresh];
}
-(void)methodChanged:(NSSegmentedControl*)sender {[appDelegate onInputTypeSelectedIndex:(int)sender.selectedSegment];[self refresh];}
-(void)resetShortcut:(id)sender {vSwitchKeyStatus=(int)0xFE0083FEu;[[NSUserDefaults standardUserDefaults] setInteger:vSwitchKeyStatus forKey:@"SwitchKeyStatus"];[self refresh];}
-(void)spellingChanged:(id)sender {vCheckSpelling=!vCheckSpelling;[[NSUserDefaults standardUserDefaults] setInteger:vCheckSpelling forKey:@"Spelling"];OnSpellCheckingChanged();RequestNewSession();[self refresh];}
-(void)smartChanged:(id)sender {vUseSmartSwitchKey=!vUseSmartSwitchKey;[[NSUserDefaults standardUserDefaults] setInteger:vUseSmartSwitchKey forKey:@"UseSmartSwitchKey"];[self refresh];}
-(void)macros:(id)sender {[appDelegate onMacroSelected];}
-(void)convert:(id)sender {[appDelegate onConvertTool];}
-(void)advanced:(id)sender {[appDelegate onAdvancedSettings];}
@end
