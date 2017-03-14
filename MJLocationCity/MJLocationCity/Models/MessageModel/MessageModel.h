//
//  MessageModel.h
//  MJLocationCity
//
//  Created by 马家俊 on 16/8/1.
//  Copyright © 2016年 MJ. All rights reserved.
//

#import <Foundation/Foundation.h>
typedef NS_ENUM(NSInteger, MessageType) {
    MessageTypeNomorl = 0,          //普通文字消息
    MessageTypePicture = 1,         //图片消息
    MessageTypeVedio = 2,           //小视频消息
};

@interface MessageModel : NSObject
{
    NSString*   m_id;
    NSInteger   m_type;         //枚举（MessageType)
    NSString*   m_content;      //内容（url）
    NSString*   m_time;
    BOOL        m_isRead;       //是否已读      pending
}
@property (nonatomic, strong) NSString*   m_id;
@property (nonatomic, strong) NSString*   m_content;
@property (nonatomic, strong) NSString*   m_time;
@property (nonatomic, assign) NSInteger   m_type;
@end
