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

        $urlRouterProvider.otherwise('/');
    }])
;
