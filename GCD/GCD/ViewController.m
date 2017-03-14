//
//  ViewController.m
//  GCD
//
//  Created by 马家俊 on 16/7/8.
//  Copyright © 2016年 MJ. All rights reserved.
//

#import "ViewController.h"
#import "bmtpParkIOS.h"
@interface ViewController ()
@property(nonatomic,strong) bmtpParkIOS * m_bmtpParkIOS;
@end

@implementation ViewController
@synthesize m_bmtpParkIOS;
- (void)viewDidLoad {
    [super viewDidLoad];
//    m_bmtpParkIOS= [[bmtpParkIOS alloc]init];
//    [m_bmtpParkIOS initWithIp:@"192.168.1.52" andPort:@"1039"];
//    [m_bmtpParkIOS sendMessage:[@"12345" UTF8String] length:21];
    // Do any additional setup after loading the view, typically from a nib.
}

- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

@end
