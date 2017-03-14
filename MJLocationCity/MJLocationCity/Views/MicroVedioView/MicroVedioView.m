//
//  MicroVedioView.m
//  MJLocationCity
//
//  Created by 马家俊 on 16/8/1.
//  Copyright © 2016年 MJ. All rights reserved.
//

#import "MicroVedioView.h"

@interface MicroVedioView()<UITableViewDelegate,UITableViewDataSource>
{
    UITableView * m_tableView;
    NSMutableArray* m_contentArr;
}
@property (nonatomic, strong) IBOutlet UITableView* p_tableView;
@property (nonatomic, strong) NSMutableArray* p_contentArr;
@end
@implementation MicroVedioView
@synthesize p_tableView = m_tableView;
@synthesize p_MicroVedioDelegate = m_MicroVedioDelegate;
@synthesize p_contentArr = m_contentArr;

-(NSMutableArray*)p_contentArr
{
    if (!m_contentArr) {
        m_contentArr = [NSMutableArray new];
    }
    return m_contentArr;
}
-(void)refreshMicroVedioView:(NSMutableArray*)contentArr
{
    self.p_contentArr = contentArr;
    [self.p_tableView reloadData];
}

-(NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section
{
    return 1;
}

-(NSInteger)numberOfSectionsInTableView:(UITableView *)tableView
{
    return self.p_contentArr.count;
}

-(UITableViewCell*)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath
{
    static NSString* Celled = @"Celled";
    UITableViewCell* cell = [tableView dequeueReusableCellWithIdentifier:Celled];
    if (!cell) {
        cell = [[UITableViewCell alloc]initWithStyle:UITableViewCellStyleValue1 reuseIdentifier:Celled];
        cell.backgroundColor = [UIColor clearColor];
    }
    NSDictionary* dic = [self.p_contentArr objectAtIndex:indexPath.section];
    cell.textLabel.text = [dic valueForKeyPath:@"content"];
    return cell;
}

-(CGFloat)tableView:(UITableView *)tableView heightForRowAtIndexPath:(NSIndexPath *)indexPath
{
    NSDictionary* dic = [self.p_contentArr objectAtIndex:indexPath.section];
    NSString* content = [dic valueForKeyPath:@"content"];
    CGSize titleSize = [content boundingRectWithSize:CGSizeMake(1000000, 13) options:NSStringDrawingUsesLineFragmentOrigin attributes:@{NSFontAttributeName:[UIFont fontWithName:@"STHeitiTC-Light" size:13]} context:nil].size;
    NSInteger numLine = titleSize.width/([[UIScreen mainScreen] bounds].size.width/2*3);
    return 20+numLine*titleSize.height;
}

@end
