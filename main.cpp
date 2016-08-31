
#include <cstdint>
#include <cstdio>
#include <quan/joystick.hpp>
#include <quan/key_was_pressed.hpp>

// show joystick channels
void print_channels(quan::joystick const & js)
{
   for (uint8_t i = 0; i < js.num_channels; ++i){
     if ( i > 0){
        printf(", ");
     }
     printf("Ch[%i] = %+6i",static_cast<int>(i) + 1, js.get_channel(i)) ;
   }
   printf("\r");
   fflush(stdout);
}

int main()
{
   try{
      printf(" --------- Taranis Joystick ... Try moving Taranis joysticks (Press any keyboard key to quit) ----------\n\n");

      quan::joystick js{"/dev/input/js0"};
      while(js.is_running() && ! quan::key_was_pressed()){
         print_channels(js);
         std::this_thread::sleep_until(std::chrono::system_clock::now() + std::chrono::milliseconds{10});
      }
      printf ("\n\n --------- Taranis Joystick Finished-----------\n");

   }catch (std::exception  & e){
      printf ("\n\nException : %s\n", e.what() );
   }
}
