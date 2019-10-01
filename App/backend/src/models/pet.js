const {Schema, model} = require('mongoose');

const DevSchema = new Schema({
    nomePet: {
        type: String,
        required:false,
    },
    nomeDono: {
        type: String,
        required:true,
    },
    userDono:{
        type: String,
        required: true,
    },
    fotoPet:{
        type: String,
        required: true,
    }
    }, {
        timestamps: true,
});

module.exports= model('Pet', DevSchema);