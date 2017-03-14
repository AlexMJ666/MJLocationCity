//
//  MicroVedioCell.h
//  MJLocationCity
//
//  Created by 马家俊 on 16/8/1.
//  Copyright © 2016年 MJ. All rights reserved.
//

#import <UIKit/UIKit.h>
#import "MessageModel.h"
@interface MicroVedioCell : UITableViewCell
{
    UIImageView* bkImageView;
    UILabel* textLabel;
}
@property (nonatomic, strong) UIImageView* bkImageView;
@property (nonatomic, strong) UILabel* textLabel;
-(void)refreshMicroVedioCell:(MessageModel*)model;
@end
