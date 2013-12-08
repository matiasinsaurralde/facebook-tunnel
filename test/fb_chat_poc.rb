#!/usr/bin/env ruby
# encoding: utf-8
# poc para usar el fb chat!

## curl -A "Mozilla Firefox" -s -b cookies.txt "https://m.facebook.com/messages/send/?icm=1&refid=12" -d "fb_dtsg=AQANMR10&body=test3&send=Responder&tids=mid.1361727138483:223bbd9ac2e4266071&wwwupp=V3&ids[100005347350787]=100005347350787"

##  sin thread ids:
##  curl -A "Mozilla Firefox" -s -b cookies.txt "https://m.facebook.com/messages/send/?icm=1&refid=12" -d "fb_dtsg=AQANMR10&body=test_sin_tids&send=Responder&wwwupp=V3&ids[100005347350787]=100005347350787"


require 'nokogiri'

$dest = 'jotajotajotajotajotajotajotajotajotajota'

def get( url )
  return `curl -A "Mozilla Firefox" -b cookies.txt -s "#{url}"`
end

def post( url, data )
  composer_url = 'https://m.facebook.com/messages/send/?icm=1&refid=12'

  data.merge!( { 'fb_dtsg' => 'AQANMR10',
                 'send'	   => 'Responder',
		 'wwwupp'   => 'V3' } )

  data_s = data.map { |k| "#{k[0]}=#{k[1]}" }.join('&')

  `curl -A "Mozilla Firefox" -s -b cookies.txt "#{composer_url}" -d "#{data_s}"`
end

def get_thread_url( dest )

  profile_body = get("https://m.facebook.com/#{dest}")
  thread_url = nil

  Nokogiri::HTML( profile_body ).css('a').each do |anchor|
    href = anchor.attr('href').to_s
    thread_url = href if href.include?( 'messages/thread' )
  end

  return thread_url

end

def send_msg( message, thread )
  composer_url, tid = "https://m.facebook.com/messages/send/?icm=1&refid=12", thread.split('/')[3]
  post( composer_url, { 'body' => message, "ids[#{tid}]" => tid } )
end

th = get_thread_url( $dest )
# th = "/messages/thread/100005347350787/?refid=17"

send_msg( 'mensaje enviado gracias a curl:D', th )
