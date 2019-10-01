const express = require('express');

const routes = express.Router();

const PetController = require ('./controllers/PetController');


routes.post('/pets', PetController.store);

module.exports = routes;