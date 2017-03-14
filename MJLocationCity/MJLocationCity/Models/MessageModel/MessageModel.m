//
//  MessageModel.m
//  MJLocationCity
//
//  Created by 马家俊 on 16/8/1.
//  Copyright © 2016年 MJ. All rights reserved.
//

#import "MessageModel.h"

@implementation MessageModel
@synthesize m_content;
@synthesize m_time;
@synthesize m_id;
@synthesize m_type;
-(id)init
{
    self = [super init];
    if (self) {
        m_id = @"";
        m_time = @"";
        m_content = @"";
        m_type = MessageTypeNomorl;
        m_isRead = NO;
    }
    return self;
}

-(id)parseFromDic:(NSDictionary *)dic
{
    if ([self init]) {
        
    }
    return self;
}

@end
