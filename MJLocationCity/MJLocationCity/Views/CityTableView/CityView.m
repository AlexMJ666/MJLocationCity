//
//  CityView.m
//  MJLocationCity
//
//  Created by 马家俊 on 16/6/28.
//  Copyright © 2016年 MJ. All rights reserved.
//

#import "CityView.h"
#import "ChineseString.h"
#import "CoreLocation/CoreLocation.h"
@interface CityView()<UITableViewDelegate,UITableViewDataSource,CLLocationManagerDelegate>
{
    CLLocationManager* locationManager;
}
@property (strong, nonatomic) CLLocationManager* locationManager;
@end
@implementation CityView
@synthesize data;
@synthesize provience;
@synthesize locationManager;
//开始定位
-(void)startLocation{
    self.locationManager = [[CLLocationManager alloc] init];
    self.locationManager.delegate = self;
    self.locationManager.desiredAccuracy = kCLLocationAccuracyBest;
    self.locationManager.distanceFilter = 10.0f;
    [self.locationManager requestAlwaysAuthorization];
    [self.locationManager startUpdatingLocation];
}

//定位代理经纬度回调
-(void)locationManager:(CLLocationManager *)manager didUpdateToLocation:(CLLocation *)newLocation fromLocation:(CLLocation *)oldLocation {
    
    [locationManager stopUpdatingLocation];
    NSLog(@"location ok");
    
    NSLog(@"%@",[NSString stringWithFormat:@"经度:%3.5f\n纬度:%3.5f",newLocation.coordinate.latitude,newLocation.coordinate.longitude]);
    
    CLGeocoder * geoCoder = [[CLGeocoder alloc] init];
    [geoCoder reverseGeocodeLocation:newLocation completionHandler:^(NSArray *placemarks, NSError *error) {
        for (CLPlacemark * placemark in placemarks) {
            
            NSDictionary *test = [placemark addressDictionary];
            //  Country(国家)  State(城市)  SubLocality(区)
            NSLog(@"%@", [test objectForKey:@"City"]);
            NSArray* arr = [NSArray arrayWithObjects:[test objectForKey:@"City"], nil];
            [data replaceObjectAtIndex:1 withObject:arr];
            [self.p_tableView reloadData];
        }
    }];
    
}

-(void)refreshTableView
{
    provience = [NSMutableArray new];
    for(char c ='A';c<='Z';c++)
    {
        NSString *zimu=[NSString stringWithFormat:@"%c",c];
        if (![zimu
              isEqualToString:@"I"]&&![zimu
                                       isEqualToString:@"O"]&&![zimu
                                                                isEqualToString:@"U"]&&![zimu isEqualToString:@"V"])
        {
            [provience addObject:[NSString stringWithFormat:@"%c",c]];
        }
    }
    
    NSString *plistPath = [[NSBundle mainBundle] pathForResource:@"cityData" ofType:@"plist"];
    NSDictionary *dictionary=[[NSDictionary alloc]initWithContentsOfFile:plistPath];
    NSArray* arr = [[NSMutableArray alloc] initWithArray:[dictionary allKeys]];
    NSMutableArray* muArr= [NSMutableArray new];
    [arr enumerateObjectsUsingBlock:^(NSString* obj, NSUInteger idx, BOOL * _Nonnull stop) {
        NSMutableArray* dicSubArr = [[NSMutableArray alloc]initWithArray:[dictionary valueForKeyPath:obj]];
        [dicSubArr removeObjectAtIndex:0];
        [muArr addObjectsFromArray:dicSubArr];
    }];
    
    provience = [ChineseString IndexArray:muArr];
    
    data = [ChineseString LetterSortArray:muArr];
    
    [self.p_tableView reloadData];
    [self startLocation];
}
-(NSInteger)numberOfSectionsInTableView:(UITableView *)tableView
{
    return provience.count;
}

-(NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section
{
    return [[data objectAtIndex:section] count];
}

-(UITableViewCell*)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath
{
    static NSString* celled = @"locationCell";
    
    UITableViewCell* cell = [tableView dequeueReusableCellWithIdentifier:celled];
    if (!cell) {
        cell = [[UITableViewCell alloc]initWithStyle:UITableViewCellStyleDefault reuseIdentifier:celled];

    }
    cell.textLabel.text = [[data objectAtIndex:indexPath.section] objectAtIndex:indexPath.row];
    return cell;
}

-(CGFloat)tableView:(UITableView *)tableView heightForRowAtIndexPath:(NSIndexPath *)indexPath
{
    return 44;
}

-(CGFloat)tableView:(UITableView *)tableView heightForHeaderInSection:(NSInteger)section
{
    return 20;
}

- (NSArray *)sectionIndexTitlesForTableView:(UITableView *)tableView
{
    return provience;
}

- (NSString *)tableView:(UITableView *)tableView titleForHeaderInSection:(NSInteger)section{
    return [provience objectAtIndex:section];
    
}


@end
