.. |sdkname| replace:: Astra

***********************
4.2 Simple Depth Reader
***********************
*Time Required: ~10 minutes*

Thirsting for more knowledge after finishing the Hello World Tutorial? Now that you've mastered some of the basic concepts of |sdkname|, let's read the depth stream from our Astra using another |sdkname| feature.

By the end of this tutorial you should be familiar with:

- The purpose of the ``FrameListener`` class
- How to define a ``FrameListener``
- Using a ``FrameListener`` to process a depth stream

Before We Begin
===============
#. Download and decompress the latest |sdkname| SDK, if you haven't already.
#. Using your favorite IDE, set up a new console application project and create a new source file called "main.cpp".
#. Copy the following into your main.cpp file:

.. code-block:: c++
   :linenos:

   #include <astra/astra.hpp>
   // for std::printf
   #include <cstdio>

   int main(int argc, char** argv)
   {
      astra::initialize();

      astra::StreamSet streamSet;
      astra::StreamReader reader = streamSet.create_reader();

      reader.stream<astra::DepthStream>().start();

      // Your code will go here

      astra::terminate();

      return 0;
   }

- Line 7 - Initializes |sdkname|
- Line 9 - Constructs a ``StreamSet``
- Line 10 - Creates a ``StreamReader``
- Line 12 - Starts a depth stream
- Line 16 - Terminates |sdkname|

Listening to Streams
====================
In the Hello World tutorial, we processed a stream of frames by looping over a call to our ``StreamReader``'s ``get_latest_frame`` function. This solution works perfectly fine in a simple case such as our Hello World application. But, what if we wanted to register for a number of streams and work with them? Or, what if we were working with more than one ``StreamSet``, or possibly more than one ``StreamReader`` per ``StreamSet``? In all of those cases, the code within the loop could quickly become complex, cluttered and cumbersome.

To alleviate these issues, |sdkname| provides us with a framework to define and create ``FrameListener`` s. A ``FrameListener`` has one function called ``on_frame_ready`` that (you guessed it!) is called when a new frame of a specific type is ready for processing. So, instead of looping over our ``StreamReader``'s ``get_latest_frame`` function, our listener will have the latest frame automatically delivered to it as soon as the frame is ready. Neato!

In order to use a ``FrameListener`` with our example...

1. We need to define a listener class that implements ``FrameListener``. This class will give us access to the actual frames that are coming from the Astra sensor. We'll get those frames in the ``on_frame_ready`` function. Copy the following code below your ``#include`` directives and above your ``main`` function:

.. code-block:: c++
   :linenos:
   :lineno-start: 7

   class DepthFrameListener : public astra::FrameListener
   {
   public:
      DepthFrameListener(int maxFramesToProcess)
          : maxFramesToProcess_(maxFramesToProcess)
      {}

      bool is_finished() const { return isFinished_; }

   private:
      void on_frame_ready(astra::StreamReader& reader,
                          astra::Frame& frame) override
      {
          const astra::DepthFrame depthFrame = frame.get<astra::DepthFrame>();

          if (depthFrame.is_valid())
          {
              print_depth_frame(depthFrame);
              ++framesProcessed_;
          }

          isFinished_ = framesProcessed_ >= maxFramesToProcess_;
      }

      void print_depth_frame(const astra::depthframe& depthFrame) const
      {
          const int frameIndex = depthFrame.frame_index();
          const short middleValue = get_middle_value(depthFrame);

          std::printf("Depth frameIndex: %d value: %d \n", frameIndex, middleValue);
      }

      short get_middle_value(const astra::depthframe& depthFrame) const
      {
          const int width = depthFrame.width();
          const int height = depthFrame.height();

          const size_t middleIndex = ((width * (height / 2.f)) + (width / 2.f));

          const short* frameData = depthFrame.data();
          const short middleValue = frameData[middleIndex];

          return middleValue;
      }

      bool isFinished_{false};
      int framesProcessed_{0};
      int maxFramesToProcess_{0};
   };

   int main(int argc, char** argv)
   {

- Line 10 - Constructor parameter specifies the total number of frames we're going to process before exiting our loop
- Line 14 - ``is_finished`` will be used in a later step to check whether we've looped the maximum number of times or not
- Line 20 - Gets the depth frame data from our frame
- Line 22 - Check to verify that we received a valid frame
- Line 24 - Prints depth frame information to the console
- Line 44 - Calculates the index of the middle pixel in our depth frame's data
- Line 47 - Gets the value of the middle depth frame pixel

.. note::

   The only required function is the ``on_frame_ready`` function. The other functions in this class support what we do within that function.

2. With the ``DepthFrameListener`` defined, let's construct our listener in the ``main`` function and add it to the ``StreamReader`` that we created in a previous step.

.. code-block:: c++
   :linenos:
   :lineno-start: 63
   :emphasize-lines: 10,11,13,17

   int main(int argc, char** argv)
   {
      astra::initialize();

      astra::StreamSet streamSet;
      astra::StreamReader reader = streamSet.create_reader();

      reader.stream<astra::DepthStream>().start();

      int maxFramesToProcess = 100;
      DepthFrameListener listener(maxFramesToProcess);

      reader.add_listener(listener);

      // More of your code will go here

      reader.remove_listener(listener);

      astra::terminate();

      return 0;
   }

- Line 73 - Constructs a ``DepthFrameListener`` that will loop 100 times
- Line 75 - Adds the listener to our reader
- Line 79 - Removes the listener from our reader

Updating our listener
======================

We've got |sdkname| and the ``StreamSet`` running, and we're listening to depth frames as they stream in through the ``StreamSet``'s ``StreamReader``. We don't know when frames are going to arrive from our Astra, so we need to continuously update those listeners by calling ``astra_temp_update`` in a loop.

.. code-block:: c++
   :linenos:
   :lineno-start: 63
   :emphasize-lines: 15-17

   int main(int argc, char** argv)
   {
      astra::initialize();

      astra::StreamSet streamSet;
      astra::StreamReader reader = streamSet.create_reader();

      reader.stream<astra::DepthStream>().start();

      const int maxFramesToProcess = 100;
      depthframeListener listener(maxFramesToProcess);

      reader.add_listener(listener);

      do {
         astra_temp_update();
      } while (!listener.is_finished());

      reader.remove_listener(listener);

      astra::terminate();

      return 0;
   }

- Line 77-79 - The |sdkname| update loop.

Let's compile and run our solution. After you've watched some depth frame information print to the console, revel in the knowledge that you've mastered the listener along with other core |sdkname| functionality. Now, go forth, let your imagination run wild and use |sdkname| to do all sorts of innovative things!
