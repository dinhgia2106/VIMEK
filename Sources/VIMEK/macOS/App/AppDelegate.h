// VIMEK — Vietnamese input method.
//  Copyright © 2019 Tuyen Mai. All rights reserved.
// SPDX-License-Identifier: GPL-3.0-only
#import <Cocoa/Cocoa.h>
#import "ViewController.h"

#define VIMEK_BUNDLE @"org.vimek.inputmethod"

@interface AppDelegate : NSObject <NSApplicationDelegate>

-(void)onImputMethodChanged:(BOOL)willNotify;
-(void)onInputMethodSelected;

-(void)askPermission;

-(void)onInputTypeSelectedIndex:(int)index;
-(void)onCodeTableChanged:(int)index;

-(void)setRunOnStartup:(BOOL)val;
-(void)loadDefaultConfig;

-(void)setGrayIcon:(BOOL)val;

-(void)onMacroSelected;
-(void)onQuickConvert;
-(void)setQuickConvertString;

-(void)showIconOnDock:(BOOL)val;
-(void)onAdvancedSettings;
-(void)onConvertTool;
-(void)onAboutSelected;
@end

