# iossimdjson
Experimenting with JSON decoding on ios 

## Requirements

- A recent mac with Xcode installed
- A recent iOS device


## Usage

- Open simdjson.xcodeproj in Xcode (macOS) 
- Click on the "simdjson" project within Xcode, go to "Identity" and select a valid account under "Team".
- Plug your device (e.g., iPhone) in your mac.
- In Xcode, go to Window > Devices. This will open a new window. You should see your device in this new window. Don't lose this new window.
- In Xcode, go to Product > Destination and choose your device. Note that a simulator won't do.
- In Xcode, go to Product > Build For > Running.
- You should see in Xcode under Products "simdjson.app", drag and drop it into the "Devices" window we opened previously.
- You should now see the app on your device. By default, iOS won't let you run it, so go under Settings > General. You will find a setting there to allow you to run the app.
- Run the app.

## Sample result

iPhone XR, Xcode 10.1
```
 gsoc-2018.json 
speed of 1.625 GB/s 
speed of 1.702 GB/s 
speed of 1.747 GB/s 
Processor: 2.496610 GHz 

 twitter.json 
speed of 1.209 GB/s 
speed of 1.285 GB/s 
speed of 1.297 GB/s 
Processor: 2.496610 GHz 

 github_events.json 
speed of 0.984 GB/s 
speed of 1.163 GB/s 
speed of 1.212 GB/s 
Processor: 2.496610 GHz 

 update-center.json 
speed of 1.030 GB/s 
speed of 1.063 GB/s 
speed of 1.101 GB/s 
Processor: 2.496610 GHz 
```

For comparison, let us multiply all these numbers by 3.7/2.5 to 'scale them' to a desktop processor.
We get speeds of 2.5 GB/s, 1.9 GB/s, 1.8 GB/s, 1.6 GB/s.
Here are the numbers we get on a skylake processor running at 3.7 GHz: 
3.2 GB/s, 2.2 GB/s, 2.4 GB/s, 1.9 GB/s.
That's unfair, however, because the skylake processor benefits from 256-bit registers (AVX).
Let us 'downgrade' the skylake processor by disabling AVX instructions, we then 
get speeds of 1.9 GB/s, 1.4 GB/s, 1.6 GB/s, 1.3 GB/s. So with a corrected clock, we find
that Apple's A12 processor is somewhere between a skylake processor with AVX disabled and
a full skylake processor, with the caveat that we have to 'scale' the frequency of the iPhone processor
artificially.



## Credit


The app. reuses code by Benoît Maison.
