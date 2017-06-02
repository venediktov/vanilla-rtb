'use strict';

angular.module('budgetsApp')

    .controller('JumboController', ['$scope', 'budgetFactory', 'budgetIdCall', function($scope, budgetFactory, budgetIdCall) {

            $scope.date = new Date();

            $scope.sortType     = 'id'; // set the default sort type
            $scope.sortReverse  = false;  // set the default sort order
            $scope.searchBudgets   = '';     // set the default search/filter term


          // create the list of campaign budgets
          //   $scope.budgets = budgetFactory.getBudgets();


            //ok create list of campaign budgets but this time with REST
            $scope.showMenu = false;
            $scope.message = "Loading ... Attempting REST .query call on budget resource";
            budgetFactory.getBudgets().query(
                function(response) {
                    $scope.budgets = response;
                    $scope.showMenu = true;
                },
                function(response) {
                                $scope.message = "Error: "+response.status + ' Could not GET budget resource from the server... Here is a mock campaign budgets object to test around with' + response.statusText;
                                $scope.budgets = [
                    { budget: 10000, cpc: 40, cpm: 5000, id: 123, spent: 5000 },
                    { budget: 200, cpc: 80, cpm: 30000, id: 456, spent: 30000 }
                ];
                            });

            //options for Price Metrics:
            $scope.metrics = ["CPM", "CPC", "CPA"];


            //code for add a budget button

            $scope.mybudget = {id: 0, cpc: 0, cpm: 0, budget: 0, spent: 0};

            $scope.addBudget = function () {

                $scope.mybudget.id = $scope.budgets[$scope.budgets.length - 1].id + 1;
                $scope.budgets.push($scope.mybudget);
                budgetIdCall.getBudget().new({id:$scope.mybudget.id}, $scope.mybudget);


                // $scope.commentForm.$setPristine();

                $scope.mybudget = {id: 0, cpc: 0, cpm: 0, budget: 0, spent: 0};
            };

            //code for delete a budget button

            $scope.deleteBudget = function (Budget) {

                if (confirm("ARE YOU SURE YOU WANT TO DELETE THIS BUDGET PLAN?") == true) {
                    $scope.removeIndex = $scope.budgets.map(function (item) {
                    return item.id;
                }).indexOf(Budget.id);
                ~$scope.removeIndex && $scope.budgets.splice($scope.removeIndex, 1);
                 budgetIdCall.getBudget().delete({id:Budget.id}, Budget);

                } else {
                    console.log(Budget);
                console.log($scope.budgets);
                }


            };

            // code for updating Campaign Budget Object

            $scope.updateObject = function () {
                console.log($scope.mybudget);
                console.log($scope.budgets);

                budgetFactory.getBudgets().update($scope.budgets);


            };



            $scope.submitBudget = function (Budget) {
                console.log(JSON.stringify(Budget));
                $scope.budget = budgetIdCall.getBudget().get({id:Budget.id});
                budgetIdCall.getBudget().update({id:Budget.id},Budget);
            };



        }]);