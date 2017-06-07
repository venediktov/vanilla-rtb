'use strict';

angular.module('budgetsApp', ['ui.router','ngResource'])
.config(['$stateProvider', '$urlRouterProvider', function($stateProvider, $urlRouterProvider) {
        $stateProvider
        
            // route for the home page
            .state('app', {
                url:'/',
                views: {

                    'content': {
                        templateUrl : 'views/header.html',
                        controller  : 'JumboController'
                    }

                }

            });
        
            // // route for the aboutus page
            // .state('app.aboutus', {
            //     url:'aboutus',
            //     views: {
            //         'content@': {
            //             templateUrl : 'views/aboutus.html',
            //             controller  : 'AboutController'
            //         }
            //     }
            // })
            //
            // // route for the contactus page
            // .state('app.contactus', {
            //     url:'contactus',
            //     views: {
            //         'content@': {
            //             templateUrl : 'views/contactus.html',
            //             controller  : 'ContactController'
            //         }
            //     }
            // })
            //
            // // route for the menu page
            // .state('app.menu', {
            //     url: 'menu',
            //     views: {
            //         'content@': {
            //             templateUrl : 'views/menu.html',
            //             controller  : 'MenuController'
            //         }
            //     }
            // })
            //
            //
            //     // route for the menu#menu id in menu page
            // .state('app.menuid', {
            //     url: 'menu#menu',
            //     views: {
            //         'content@': {
            //             templateUrl : 'views/menu.html',
            //             controller  : 'MenuController'
            //         }
            //     }
            // })
            //
            //
            //
            // // route for the dishdetail page
            // .state('app.dishdetails', {
            //     url: 'menu/:id',
            //     views: {
            //         'content@': {
            //             templateUrl : 'views/dishdetail.html',
            //             controller  : 'DishDetailController'
            //        }
            //     }
            // });
    
        $urlRouterProvider.otherwise('/');
    }])
;
