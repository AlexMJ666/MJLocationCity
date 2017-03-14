//
//  bmtpParkIOS.h
//  GCD
//
//  Created by 马家俊 on 16/8/8.
//  Copyright © 2016年 MJ. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "bmtp.h"
@interface bmtpParkIOS : NSObject

-(void)initWithIp:(NSString *)ip andPort:(NSString *)port;
-(void)clean;
-(void)sendMessage:(const char *)data length:(int)len;
@end
