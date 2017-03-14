//
//  ;
//  MJLocationCity
//
//  Created by 马家俊 on 16/8/1.
//  Copyright © 2016年 MJ. All rights reserved.
//

#import <UIKit/UIKit.h>

@protocol MicroVedioDelegate <NSObject>


@end

@interface MicroVedioView : UIView
{
    __weak id<MicroVedioDelegate> m_MicroVedioDelegate;
    
}
@property (nonatomic, weak) id<MicroVedioDelegate> p_MicroVedioDelegate;
-(void)refreshMicroVedioView:(NSMutableArray*)contentArr;
@end
