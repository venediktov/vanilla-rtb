'use strict';

angular.module('budgetsApp')

    .controller('JumboController', ['$scope', 'budgetFactory', function($scope, budgetFactory) {

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


                $scope.budgets.push($scope.mybudget);
                budgetFactory.getBudgets().save($scope.mybudget);


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
                 budgetFactory.getBudgets().delete(Budget);

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

                // ok this is key i think i need to make an instance of budget then get budgets, and .update by selected id:00, update the whole scope.budget... will try in the morning tomorrow.

                // budgetFactory.getDishes().update({id:$scope.dish.id},$scope.dish);


            };



            $scope.submitBudget = function (Budget) {

                $scope.budget = budgetFactory.getBudgets().get({id:Budget.id});

                budgetFactory.getBudgets().update({id:Budget.id},Budget);

            };




            $scope.submitComment = function () {
                                $scope.mycomment.date = new Date().toISOString();
                console.log($scope.mycomment);
                                $scope.dish.comments.push($scope.mycomment);

                                //REST .UPDATE
                budgetFactory.getDishes().update({id:$scope.dish.id},$scope.dish);
                                $scope.commentForm.$setPristine();
                                $scope.mycomment = {rating:5, comment:"", author:"", date:""};
            }


        }]);
