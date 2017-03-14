//
//  Common.h
//  GCD
//
//  Created by 马家俊 on 16/8/9.
//  Copyright © 2016年 MJ. All rights reserved.
//

#import <Foundation/Foundation.h>

@interface Common : NSObject
{
    NSMutableArray* messageArr;
}
+(Common*)shareInstanceManager;
@property (nonatomic, strong)NSMutableArray* messageArr;
@end
