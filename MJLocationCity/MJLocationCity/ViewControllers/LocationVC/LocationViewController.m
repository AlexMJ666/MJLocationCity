//
//  LocationViewController.m
//  MJLocationCity
//
//  Created by 马家俊 on 16/6/7.
//  Copyright © 2016年 MJ. All rights reserved.
//

#import "LocationViewController.h"
#import "CityView.h"
@interface LocationViewController ()
{
    CityView* m_cityView;
}
@property(nonnull,strong) IBOutlet CityView* p_cityView;
@end

@implementation LocationViewController

- (void)viewDidLoad {
    [super viewDidLoad];
    [self.p_cityView refreshTableView];
    // Do any additional setup after loading the view.
}


- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

@end
