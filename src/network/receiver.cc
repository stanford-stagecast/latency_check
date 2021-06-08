#include "receiver.hh"
#include "timer.hh"

using namespace std;

template<class FrameType>
void NetworkReceiver<FrameType>::receive_sender_section(
  const typename Packet<FrameType>::SenderSection& sender_section )
{
  if ( not biggest_seqno_received_.has_value() ) {
    biggest_seqno_received_ = sender_section.sequence_number;
  } else {
    biggest_seqno_received_ = max( biggest_seqno_received_.value(), sender_section.sequence_number );
  }

  const uint64_t now = Timer::timestamp_ns();

  for ( const auto& frame : sender_section.frames ) {
    unreceived_beyond_this_frame_index_ = max( unreceived_beyond_this_frame_index_, frame.frame_index + 1 );

    if ( frame.frame_index < next_frame_needed_ ) {
      stats_.already_acked++;
      continue;
    }

    if ( frame.frame_index >= frames_.range_end() ) {
      discard_frames( frame.frame_index - frames_.range_end() + 1 );
    }

    auto& dest = frames_.at( frame.frame_index );
    if ( dest.has_value() ) {
      stats_.redundant++;
      continue;
    }

    dest = frame;
    stats_.last_new_frame_received = now;
  }

  advance_next_frame_needed();

  if ( sender_section.frames.length ) {
    if ( recent_packets_.num_stored() >= recent_packets_.capacity() ) {
      recent_packets_.pop( 1 );
    }

    recent_packets_.writable_region().at( 0 ) = sender_section.to_record();
    recent_packets_.push( 1 );
  }
}

template<class FrameType>
void NetworkReceiver<FrameType>::discard_frames( const unsigned int num )
{
  frames_.pop( num );
  stats_.dropped += num;
  next_frame_needed_ = frames_.range_begin();
  advance_next_frame_needed();
}

template<class FrameType>
void NetworkReceiver<FrameType>::advance_next_frame_needed()
{
  while ( next_frame_needed_ < frames_.range_end() and frames_.at( next_frame_needed_ ).has_value() ) {
    next_frame_needed_++;
  }
}

template<class FrameType>
void NetworkReceiver<FrameType>::set_receiver_section(
  typename Packet<FrameType>::ReceiverSection& receiver_section )
{
  receiver_section.next_frame_needed = next_frame_needed_;

  if ( biggest_seqno_received_.has_value() ) {
    receiver_section.packets_received.push_back( biggest_seqno_received_.value() );
  }

  const span_view<typename Packet<FrameType>::Record> recent = recent_packets_.readable_region();
  for ( auto it = recent.end() - 1; it >= recent.begin(); --it ) {
    const auto& p = *it;
    if ( p.sequence_number == biggest_seqno_received_ ) {
      continue;
    }

    // does the packet acknowledge a frame that isn't otherwise acknowledged?
    bool sack_the_packet = false;
    for ( const auto frame_index : p.frames ) {
      if ( frame_index == next_frame_needed_ ) {
        throw runtime_error( "BUG: packet received but frame still needed???" );
      }
      if ( frame_index > next_frame_needed_ ) {
        sack_the_packet = true;
        break;
      }
    }
    if ( sack_the_packet ) {
      receiver_section.packets_received.push_back( p.sequence_number );
      if ( receiver_section.packets_received.length >= receiver_section.packets_received.capacity ) {
        break;
      }
    }
  }
}

template<class FrameType>
void NetworkReceiver<FrameType>::summary( ostream& out ) const
{
  out << "Receiver info:";

  if ( stats_.last_new_frame_received.has_value() ) {
    out << " last_new_frame=";
    Timer::pp_ns( out, Timer::timestamp_ns() - stats_.last_new_frame_received.value() );
  }

  if ( stats_.already_acked ) {
    out << " already_acked=" << stats_.already_acked << "!";
  }
  if ( stats_.redundant ) {
    out << " redundant=" << stats_.redundant << "!";
  }
  if ( stats_.dropped ) {
    out << " dropped=" << stats_.dropped << "!";
  }

  const uint32_t contiguous_count = next_frame_needed_ - frames_.range_begin();
  uint32_t other_count = 0;
  optional<uint32_t> first_other_held;
  for ( uint32_t i = next_frame_needed_;
        i < min( uint32_t( frames_.range_end() ), unreceived_beyond_this_frame_index_ );
        i++ ) {
    if ( frames_[i].has_value() ) {
      other_count++;
      if ( not first_other_held.has_value() ) {
        first_other_held = i;
      }
    }
  }

  if ( stats_.popped ) {
    out << " popped=[0.." << stats_.popped - 1 << "]";
  }

  if ( contiguous_count ) {
    out << " contig=[" << frames_.range_begin() << ".." << next_frame_needed_ - 1 << "]";
  } else {
    out << " next_frame_needed=" << next_frame_needed_;
  }

  if ( first_other_held.has_value() ) {
    out << " + " << other_count << " other (" << first_other_held.value() << " - "
        << unreceived_beyond_this_frame_index_ - 1 << ")";
  }

  out << "\n";
}

template<class FrameType>
void NetworkReceiver<FrameType>::pop_frames( const size_t num )
{
  if ( num > next_frame_needed_ - frames_.range_begin() ) {
    throw std::out_of_range( "pop_frames: " + to_string( num ) + " > "
                             + to_string( next_frame_needed_ - frames_.range_begin() ) );
  }

  frames_.pop( num );
  stats_.popped += num;
}

template class NetworkReceiver<AudioFrame>;
