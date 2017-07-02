Biased Plugin
-------------

Biased is a UE4 plugin that can create biased dice (i.e. dice where each face can have a different probability of being rolled) and roll them, generating values, in constant time.

Usage
-------------

To achieve the constant time rolling of these dice some pre-processing is required. We being by simply describing each face of a die using a ```FDieFace``` struct, giving it a value (i.e. the pip value of the face) and a probability that this face will be rolled. An array of such faces can be used to describe a whole die.

Once we have a description of a die we need to process it into a format that our algorithm can use by passing the array into ```GenerateBiasedDieData()```. This function will process the die and give us a ```FBiasedDieData``` struct which corresponds to the die described by the input array. Note that for the generation of the ```FBiasedDieData``` to work the sum of the probabilities of the faces needs to equal 1. If you have a die where this is not the case you can use ```NormaliseDieFaces()``` to get it into the proper form.

Now we have all the data in the right format rolling the die we can do so by calling ```RollBiasedDie()``` (or ```RollBiasedDieFromStream()``` if you want to take advantage of Random Streams). ```RollBiasedDie()``` will return a value in accordance of the ```FBiasedDieData``` used as input.

```RollBiasedDie()``` will roll dice in constant time, meaning the time it takes to roll a 6 sided die will be the same time it takes for it to roll a 1000 sided die.

Everything described above is exposed to both native C++ code and Blueprint as a Blueprint Library so usage should be simple and accessible.
