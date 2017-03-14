//
//  Common.m
//  GCD
//
//  Created by 马家俊 on 16/8/9.
//  Copyright © 2016年 MJ. All rights reserved.
//

#import "Common.h"

static Common* g_common;
@implementation Common
@synthesize messageArr;

+(Common*)shareInstanceManager
{
    @synchronized (self) {
        if (!g_common) {
            g_common = [[Common alloc]init];
        }
    }
    return g_common;
}
@end
