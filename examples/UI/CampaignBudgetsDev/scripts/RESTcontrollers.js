'use strict';

angular.module('budgetsApp')

    .controller('JumboController', ['$scope', function($scope) {

            $scope.date = new Date();
        }])


        .controller('MenuController', ['$scope', 'budgetFactory', function($scope, budgetFactory) {
            


                        $scope.showMenu = false;
            $scope.message = "Loading ... Attempting REST Call";
                        budgetFactory.getDishes().query(
                function(response) {
                    $scope.dishes = response;
                    $scope.showMenu = true;
                },
                function(response) {
                                $scope.message = "Error: "+response.status + " the Menu data was not pulled by jSON-server because the server is not connected to GitHub pages." + response.statusText;
                            });



        }])



        .controller('DishDetailController', ['$scope', '$stateParams', 'budgetFactory', function($scope, $stateParams, budgetFactory) {

            var dish= budgetFactory.getDishes(parseInt($stateParams.id,10));

            $scope.showDish = false;
            $scope.message="Loading ... Attempting REST GET Call to retrieve Dish Detail Data";
            $scope.dish = budgetFactory.getDishes().get({id:parseInt($stateParams.id,10)})
            .$promise.then(
                            function(response){
                                $scope.dish = response;
                                $scope.showDish = true;
                            },
                            function(response) {
                                $scope.message = "Error: "+response.status + " Note: the Menu data was not pulled by jSON-server because server not connected to this site" + response.statusText;
                            }
            );


            
        }])

        .controller('DishCommentController', ['$scope', 'budgetFactory', function($scope,budgetFactory) {
            
            $scope.mycomment = {rating:5, comment:"", author:"", date:""};
            
             $scope.submitComment = function () {
                                $scope.mycomment.date = new Date().toISOString();
                console.log($scope.mycomment);
                                $scope.dish.comments.push($scope.mycomment);

                                //REST .UPDATE
                budgetFactory.getDishes().update({id:$scope.dish.id},$scope.dish);
                                $scope.commentForm.$setPristine();
                                $scope.mycomment = {rating:5, comment:"", author:"", date:""};
            }
        }])

        // implement the IndexController and About Controller here
        .controller('IndexController', ['$scope', 'budgetFactory', 'corporateFactory', function($scope, budgetFactory, corporateFactory) {

           $scope.leader = corporateFactory.getLeader(0);

            $scope.promotion = {};
            budgetFactory.getPromotion(0)
                        .then(
                            function(response){
                                $scope.promotion = response.data;
                                $scope.showDish = true;
                            }
                        );
                        $scope.showDish = false;
                        $scope.message="Loading ... Attempting REST Call GET to pull home page content";
                        $scope.dish = budgetFactory.getDishes().get({id:3})
                        .$promise.then(
                            function(response){
                                $scope.dish = response;
                                $scope.showDish = true;
                            },
                            function(response) {
                                $scope.message = "Error: "+response.status + " The .json file associated with the content of this site was unable to be served by GitHub pages, will be working on implementing solution soon. " + response.statusText;
                            }
                        );


        }])
;
