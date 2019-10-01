const axios = require('axios');
const Pet = require ('../models/Pet');


module.exports = {
    async store(req, res){
        const { username } = req.body;

        const userExists = await Pet.findOne({userDono : username});

        if (userExists){ // se usuário já existir na base
            console.log('Já existe na base!');
            return res.json(userExists);
        }

        const response = await axios.get(`https://api.github.com/users/${username}`);
        const {name, avatar_url} = response.data;
        
        const pet = await Pet.create(
            {
                nomeDono: name,
                userDono: username,
                fotoPet: avatar_url 
            }
        );

        return res.json(pet);
    }
};