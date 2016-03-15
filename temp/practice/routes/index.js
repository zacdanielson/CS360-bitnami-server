var express = require('express');
var fs = require('fs');
var router = express.Router();

/* GET home page. */
router.get('/', function(req, res, next) {
  res.render('index', { title: 'Express' });
});
router.get('/getcity',function(req,res,next){
	console.log("in getcity route");
	fs.readFile(__dirname + '/cities.dat.txt',function(err,data) { 
		if(err) throw err; 
		var cities = data.toString().split("\n"); 
		var jsonresult = [];
		var myRe = new RegExp("^" + req.query.q);
		for(var i = 0; i < cities.length; i++) { 
			var result = cities[i].search(myRe);
			if(result !=-1) {
				console.log(cities[i]);
				jsonresult.push({city:cities[i]});
		}
		} 
		res.status(200).json(jsonresult);
	})


});

module.exports = router;
