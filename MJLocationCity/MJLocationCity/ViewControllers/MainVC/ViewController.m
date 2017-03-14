//
//  ViewController.m
//  MJLocationCity
//
//  Created by 马家俊 on 16/6/7.
//  Copyright © 2016年 MJ. All rights reserved.
//

#import "ViewController.h"

@interface ViewController ()

@end

@implementation ViewController

- (void)viewDidLoad {
    [super viewDidLoad];
    // Do any additional setup after loading the view, typically from a nib.
}

- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}
- (IBAction)showLocation:(id)sender {
    [self performSegueWithIdentifier:@"showlocation" sender:nil];
}

@end
