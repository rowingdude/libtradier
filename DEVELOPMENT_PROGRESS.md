## Development Notes

As this project has progressed and I have learned more and more about programming in high performance environments, I have tried to include these concepts into this library. 

To that end, I have "abandoned all hope" and embraced LibBoost, which I was previously against, however benchmarks bear fruit and I'm not going to argue for bespoke software where it needn't be. 

The next major update is the replacement of nlohmann-json with simdJson. I was familiar with this project, but waited to adopt it until it was a more stable iteration, as it's now quite stable and in testing 13x more performant, it is now the JSON processor. 

As a result of these, I have removed all references and code pertaining to websocketpp and nlohmann json. 
