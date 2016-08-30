
#include <cstdint>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/joystick.h>
#include <stdexcept>
#include <thread>    
#include <chrono>  

/*
 joystick class connected to usb
*/     
       
struct joystick{
   static constexpr uint8_t num_channels = 8;
   joystick(const char* filename) // e.g filename = "/dev/input/js0"
   {
      for (auto & v : m_channel){v = 0;}

      m_fd = open(filename, O_RDONLY | O_NONBLOCK);
      if (m_fd < 0){
         throw std::runtime_error("failed to open joystick file");
      }
      m_update_thread = std::thread{std::bind(&joystick::update,this)};
   }
   ~joystick(){ if (m_fd >= 0) { close(m_fd);} }

   int32_t get_channel(uint8_t channel)const
   {
      if ( channel < num_channels){
         return m_channel[channel];
      }else{
         return 0;
      }
   }

private:
   void update()
   {
      for(;;){
         while (read (m_fd, &m_event, sizeof(m_event)) == sizeof(m_event)) {
            if ((m_event.type & JS_EVENT_AXIS)==JS_EVENT_AXIS){
               set_channel();
            }
         }
         if (errno != EAGAIN) {
            throw std::runtime_error("failed to read joystick");
         }
         std::this_thread::sleep_until(std::chrono::system_clock::now() + std::chrono::milliseconds{10});
      }
   }

   void set_channel()
   {
      if ( m_event.number < num_channels){
         m_channel[m_event.number] = m_event.value;
      }
   }

   int32_t   m_channel[num_channels];
   int       m_fd;
   js_event  m_event;
   std::thread m_update_thread;
   joystick (const joystick &)= delete;
   joystick & operator = (const joystick &)= delete;
};

// show joystick channels
void print_channels(joystick const & js)
{
   for (uint8_t i = 0; i < js.num_channels; ++i){
     if ( i > 0){
        printf(", ");
     }
     printf("Ch[%i] = %+6i",static_cast<int>(i), js.get_channel(i)) ;
   }
   printf("\r");
   fflush(stdout);
}

int main()
{
   try{
      joystick js{"/dev/input/js0"};
      for (;;){
         print_channels(js);
         std::this_thread::sleep_until(std::chrono::system_clock::now() + std::chrono::milliseconds{10});
      }
   }catch (std::exception  & e){
      printf ("\n\nException : %s\n", e.what() );
   }
}
