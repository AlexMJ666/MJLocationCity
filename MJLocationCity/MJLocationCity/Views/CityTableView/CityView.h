//
//  CityView.h
//  MJLocationCity
//
//  Created by 马家俊 on 16/6/28.
//  Copyright © 2016年 MJ. All rights reserved.
//

#import <UIKit/UIKit.h>

@interface CityView : UIView
{
    UITableView* m_tableView;
}


@property (strong, nonatomic) NSMutableArray *data;
@property (strong, nonatomic) NSMutableArray *provience;
@property (nonatomic, strong) IBOutlet UITableView* p_tableView;

-(void)refreshTableView;
@end
