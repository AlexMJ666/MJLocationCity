//
//  MicroVedioCell.m
//  MJLocationCity
//
//  Created by 马家俊 on 16/8/1.
//  Copyright © 2016年 MJ. All rights reserved.
//

#import "MicroVedioCell.h"

@implementation MicroVedioCell
@synthesize bkImageView;
@synthesize textLabel;

-(UIImageView*)bkImageView
{
    if (!bkImageView) {
        bkImageView = [UIImageView new];
        [self addSubview:bkImageView];
    }
    return bkImageView;
}

-(void)refreshMicroVedioCell:(MessageModel *)model
{
    switch (model.m_type) {
        case MessageTypeNomorl:
            [self refreshNomorl:model];
            break;
            
        default:
            break;
    }
}

-(void)refreshNomorl:(MessageModel *)model
{
    
}

- (void)awakeFromNib {
    [super awakeFromNib];
    // Initialization code
}

- (void)setSelected:(BOOL)selected animated:(BOOL)animated {
    [super setSelected:selected animated:animated];

    // Configure the view for the selected state
}

@end
