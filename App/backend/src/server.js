const express = require('express');
const mongoose = require('mongoose');
const routes = require('./routes');

const server = express();

mongoose.connect('mongodb+srv://petfinder:petfinder@clusterpetfinder-ju7xn.mongodb.net/petFinder?retryWrites=true&w=majority',
{   useUnifiedTopology: true,
    useNewUrlParser: true
});



server.use(express.json());
server.use(routes);

server.listen(3333);