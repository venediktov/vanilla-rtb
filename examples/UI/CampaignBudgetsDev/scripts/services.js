'use strict';

angular.module('budgetsApp')


        .constant("baseURL","http://localhost:3000/")
        .service('budgetFactory', ['$resource', 'baseURL', function($resource, baseURL) {

                // this.getDishes = function(){
                //     return $resource(baseURL+"dishes/:id",null,  {'update':{method:'PUT' }});
                // };

                this.getBudgets = function () {
                    return $resource(baseURL+"budgets/:id",null,  {'update':{method:'PUT'}});
                };



                // var budgets = [
                //     { budget: 10000, cpc: 40, cpm: 5000, id: 123, spent: 5000 },
                //     { budget: 200, cpc: 80, cpm: 30000, id: 456, spent: 30000 }
                // ];
                //
                // this.getBudgets = function () {
                //     return budgets;
                // };

                        
        }])



;
