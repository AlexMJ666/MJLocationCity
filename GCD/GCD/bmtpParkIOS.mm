//
//  bmtpParkIOS.m
//  GCD
//
//  Created by 马家俊 on 16/8/8.
//  Copyright © 2016年 MJ. All rights reserved.
//

#import "bmtpParkIOS.h"

@interface bmtpParkIOS()
@property (nonatomic) BMTP *bmtp;
@end
@implementation bmtpParkIOS

-(id)init
{
    self = [super init];
    if (self) {
        bmtp_init();
    }
    return self;
}

-(void)initWithIp:(NSString *)ip andPort:(NSString *)port
{

    _bmtp = bmtp_new([ip UTF8String], port.intValue);
    
    if (_bmtp) {
        //注册监听
        bmtp_set_on_open(_bmtp,on_open);
        bmtp_set_on_close(_bmtp,on_close);
        bmtp_set_on_error(_bmtp,on_error);
        bmtp_set_on_message(_bmtp,on_message);
        
        bmtp_open(_bmtp);
        
    }
}

#pragma mark - BMTP Command

-(void)clean
{
    bmtp_cleanup();
}

-(void)sendMessage:(const char *)data length:(int)len
{
    bmtp_pub(_bmtp,data,len,0);
}

#pragma mark - BMTP block
void on_open( BMTP* bmtp)
{
    //TODO channel_id
    //每次建立连接时，获取消息
    bmtp_sub(bmtp,1);
}


void on_message( BMTP* bmtp, const char *data, int len )
{
    //获取长度为len的data
    NSString *datastr = [NSString stringWithUTF8String:data];
    //    NSString *datastr = [NSString stringWithCharacters:(const unichar *)data length:len];
    NSOperationQueue *queue = [[NSOperationQueue alloc] init];
    [queue addOperationWithBlock:^{
        //解析消息
        NSLog(@"%@",datastr);
    }];
}

void on_close( BMTP* bmtp)
{
}

void on_error( BMTP* bmtp, int err_no )
{
    
}

@end
