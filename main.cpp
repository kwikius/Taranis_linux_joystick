

#include <cstdint>
#include <cstdlib>
#include <cstdio>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/joystick.h>
#include <stdexcept>
#include <thread>    
#include <chrono> 
#include <atomic> 

/*
 joystick class connected to usb
 https://www.kernel.org/doc/Documentation/input/joystick-api.txt
*/     
       
struct joystick{
   static constexpr uint8_t num_channels = 8;
   
   joystick(const char* device_name) // e.g device_name = "/dev/input/js0"
   : m_fd{open(device_name, O_RDONLY | O_NONBLOCK)}, m_thread_running{false}, m_want_thread_quit{true}
   {
      if (m_fd < 0){
         throw std::runtime_error("failed to open joystick device");
      }
      for (auto & v : m_channel){v = 0;}
      m_thread_running = true;
      m_want_thread_quit = false;
      m_update_thread = std::thread{std::bind(&joystick::update,this)};
   }
   ~joystick(){ end_thread(); if (m_fd >= 0) { close(m_fd);} }

   bool is_running() const { return m_thread_running == true;}

   int32_t get_channel(uint8_t channel)const
   {
      if ( channel < num_channels){
         return m_channel[channel];
      }else{
         return 0;
      }
   }

   void end_thread()
   {
      m_want_thread_quit = true;
      while (m_thread_running){;}
      m_update_thread.join();
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
         if( (errno != EAGAIN) || m_want_thread_quit) {
            if ( errno != EAGAIN){
               printf("joystick device read failed\n");
            }
            m_thread_running = false;
            return;
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
   std::atomic<int32_t>   m_channel[num_channels];
   std::atomic<int>       m_fd;
   std::atomic<bool>      m_thread_running;
   std::atomic<bool>      m_want_thread_quit;
   js_event               m_event;
   std::thread            m_update_thread;
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
     printf("Ch[%i] = %+6i",static_cast<int>(i) + 1, js.get_channel(i)) ;
   }
   printf("\r");
   fflush(stdout);
}

//http://stackoverflow.com/questions/22166074/is-there-a-way-to-detect-if-a-key-has-been-pressed
bool kbhit(void)
{
  termios oldt;
  tcgetattr(STDIN_FILENO, &oldt);

  termios newt = oldt;
  newt.c_lflag &= ~(ICANON | ECHO);
  tcsetattr(STDIN_FILENO, TCSANOW, &newt);

  int oldf = fcntl(STDIN_FILENO, F_GETFL, 0);

  fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

  int ch = getchar();
  bool input = ch != EOF;

  tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
  fcntl(STDIN_FILENO, F_SETFL, oldf);

  if(input)
  {
     ungetc(ch, stdin);
  }
  return input;
}

int main()
{
   try{
      printf(" --------- Taranis Joystick ... Try moving Taranis joysticks (Press any key to quit) ----------\n\n");

      joystick js{"/dev/input/js0"};
      while(js.is_running() && ! kbhit()){
         print_channels(js);
         std::this_thread::sleep_until(std::chrono::system_clock::now() + std::chrono::milliseconds{10});
      }
      printf ("\n\n --------- Finished Taranis Joystick -----------\n");

   }catch (std::exception  & e){
      printf ("\n\nException : %s\n", e.what() );
   }
}
