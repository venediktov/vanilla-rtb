'use strict';

angular.module('budgetsApp')


        .constant("baseURL","http://localhost:11081/campaign/")
        .service('budgetFactory', ['$resource', 'baseURL', function($resource, baseURL) {


                this.getBudgets = function () {
                    return $resource(baseURL+"budget/:id",null,  {'update':{method:'PUT'}});
                };


                        
        }])
        .service('budgetIdCall', ['$resource', 'baseURL', function($resource, baseURL) {
                this.getBudget = function () {
                    return $resource(baseURL+"budget/id/:id", null,
                        {
                            'update':{method:'POST', headers: { 'Content-Type': 'application/x-www-form-urlencoded'}},
                            'new':{method:'PUT'}
                        }
                    );
                };
        }])


;