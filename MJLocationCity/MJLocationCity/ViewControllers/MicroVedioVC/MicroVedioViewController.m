//
//  MicroVedioViewController.m
//  MJLocationCity
//
//  Created by 马家俊 on 16/8/1.
//  Copyright © 2016年 MJ. All rights reserved.
//

#import "MicroVedioViewController.h"
#import "MicroVedioView.h"
@interface MicroVedioViewController ()<MicroVedioDelegate>
{
    MicroVedioView* m_microVedioView;
}
@property(nonatomic,strong) IBOutlet MicroVedioView* p_microVedioView;
@end

@implementation MicroVedioViewController
@synthesize p_microVedioView = m_microVedioView;

- (void)viewDidLoad {
    [super viewDidLoad];
    self.p_microVedioView.p_MicroVedioDelegate = self;
    [self.p_microVedioView refreshMicroVedioView:[NSMutableArray arrayWithObjects:
                [NSDictionary dictionaryWithObjectsAndKeys:@"1",@"id",@"sx",@"userName",@"你好",@"content", nil],
                [NSDictionary dictionaryWithObjectsAndKeys:@"2",@"id",@"mj",@"userName",@"你好啊，最近好吗你好啊，最近好吗你好啊，最近好吗你好啊，最近好吗你好啊，最近好吗你好啊，最近好吗你好啊，最近好吗你好啊，最近好吗你好啊，最近好吗你好啊，最近好吗你好啊，最近好吗你好啊，最近好吗你好啊，最近好吗你好啊，最近好吗你好啊，最近好吗你好啊，最近好吗你好啊，最近好吗你好啊，最近好吗你好啊，最近好吗你好啊，最近好吗你好啊，最近好吗",@"content", nil],
                [NSDictionary dictionaryWithObjectsAndKeys:@"3",@"id",@"sx",@"userName",@"挺好的，你呢",@"content", nil],
                [NSDictionary dictionaryWithObjectsAndKeys:@"4",@"id",@"mj",@"userName",@"我也是，哈哈",@"content", nil],
                [NSDictionary dictionaryWithObjectsAndKeys:@"5",@"id",@"sx",@"userName",@"拜拜",@"content", nil],
                                           nil]];
    // Do any additional setup after loading the view.
}

- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

/*
#pragma mark - Navigation

// In a storyboard-based application, you will often want to do a little preparation before navigation
- (void)prepareForSegue:(UIStoryboardSegue *)segue sender:(id)sender {
    // Get the new view controller using [segue destinationViewController].
    // Pass the selected object to the new view controller.
}
*/

@end
