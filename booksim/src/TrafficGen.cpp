//
// Created by Ben on 7/19/21.
//

#include "TrafficGen.h"

TrafficGen::TrafficGen(const Configuration &config, const vector<Network *> &net) : TrafficManager(config, net) {
    _flit_width = 8;//config.GetInt( "flit_width" );
    _ideal_interconnect = 0; //config.GetInt( "ideal_interconnect" );
    _interface = new Interface(config, net);
    _interface->Init();
}

TrafficGen::~TrafficGen(){
}

void TrafficGen::_RetireFlit(Flit *f, int dest){
    ReplyPacket *p;
    _deadlock_timer = 0;

    //Ensure the flit was in the network
    assert(_total_in_flight_flits[f->cl].count(f->id) > 0);
    _total_in_flight_flits[f->cl].erase(f->id);

    if(f->record) {
        //Ensure the measured flit was in the network
        assert(_measured_in_flight_flits[f->cl].count(f->id) > 0);
        _measured_in_flight_flits[f->cl].erase(f->id);
    }
    if ( f->watch ) {
        *gWatchOut << GetSimTime() << " | "
                   << "node" << dest << " | "
                   << "Retiring flit " << f->id
                   << " (packet " << f->pid
                   << ", src = " << f->src
                   << ", dest = " << f->dest
                   << ", hops = " << f->hops
                   << ", flat = " << f->atime - f->itime
                   << ")." << endl;
    }

    if ( f->head && ( f->dest != dest ) ) {
        std::ostringstream err;
        err << "Flit " << f->id << " arrived at incorrect output " << dest;
        Error( err.str( ) );
    }
    if((_slowest_flit[f->cl] < 0) ||
       (_flat_stats[f->cl]->Max() < (f->atime - f->itime))) {
        _slowest_flit[f->cl] = f->id;
    }

    //Record flit latency stats
    _flat_stats[f->cl]->AddSample( f->atime - f->itime);
    //Flit latency flows
    _pair_flat[f->cl][f->src*_nodes+dest]->AddSample( f->atime - f->itime );

    //If this is the tail flit, notify FeS2 the packet has arrived
    if ( f->tail ) {
        p = new ReplyPacket;
        p->dest = dest;
        p->source = f->src;
        PayLoad* payload = (PayLoad*)(f->data);
        p->id = payload->id;
        p->network = payload->subnetwork;
        _interface->EnqueueReplyPacket(p);
        delete payload;
        Flit * head;
        if(f->head) {
            head = f;
        } else {
            map<int, Flit *>::iterator iter = _retired_packets[f->cl].find(f->pid);
            assert(iter != _retired_packets[f->cl].end());
            head = iter->second;
            _retired_packets[f->cl].erase(iter);
            assert(head->head);
            assert(f->pid == head->pid);
        }
        if ( f->watch ) {
            *gWatchOut << GetSimTime() << " | "
                       << "node" << dest << " | "
                       << "Retiring packet " << f->pid
                       << " (plat = " << f->atime - head->ctime
                       << ", nlat = " << f->atime - head->itime
                       // NB: In the spirit of solving problems using ugly hacks,
                       // we compute the packet length by taking advantage of the
                       // fact that the IDs of flits within a packet are contiguous
                       << ", frag = " << (f->atime-head->atime) - (f->id-head->id)
                       << ", src = " << head->src
                       << ", dest = " << head->dest
                       << ")." << endl;
        }
        //code the source of request, look carefully, its tricky ;)
        if (f->type == Flit::READ_REQUEST || f->type == Flit::WRITE_REQUEST) {
            /* Removing memory leak
            PacketReplyInfo* rinfo = PacketReplyInfo::New();
            rinfo->source = f->src;
            rinfo->time = f->atime;
            rinfo->record = f->record;
            rinfo->type = f->type;
            _repliesPending[dest].push_back(rinfo);
             */
        } else {
            if(f->type == Flit::READ_REPLY || f->type == Flit::WRITE_REPLY  ) {
                _requestsOutstanding[dest]--;
            } else if(f->type == Flit::ANY_TYPE) {
                _requestsOutstanding[f->src]--;
            }
        }
        // Only record statistics once per packet (at tail, already checked)
        // and based on the simulation state
        if ( (_sim_state == warming_up) || f->record ) {
            _hop_stats[f->cl]->AddSample( f->hops );

            // TODO Didn't we already do this?
            if((_slowest_packet[f->cl] < 0) ||
               (_plat_stats[f->cl]->Max() < (f->atime - head->itime))) {
                _slowest_packet[f->cl] = f->pid;
            }

            //Packet latency: Arrival Time - Creation Time
            _plat_stats[f->cl]->AddSample(f->atime - head->ctime);
            //Network Latency: Arrival Time - Injected Time
            _nlat_stats[f->cl]->AddSample(f->atime - head->itime);
            //??: Useless?
            _frag_stats[f->cl]->AddSample(
                    (f->atime - head->atime) - (f->id - head->id));

            // Latency flows
            _pair_plat[f->cl][f->src*_nodes+dest]
                    ->AddSample(f->atime - head->ctime);
            _pair_nlat[f->cl][f->src*_nodes+dest]
                    ->AddSample(f->atime - head->itime);
        }

        if(f != head) {
            head->Free();
        }
    }

    if(f->head && !f->tail) {
        _retired_packets[f->cl].insert(make_pair(f->pid, f));
    }
    else {
        f->Free();
    }
}

void TrafficGen::_GeneratePacket(int source, int size, int cl, int time, int id, int subnetwork, int destination) {
    assert(size!=0);
    PayLoad* payload = new PayLoad;
    payload->id = id;
    payload->subnetwork = subnetwork;
    Flit::FlitType packet_type = Flit::ANY_TYPE;
    //Packet ID
    int pid = _cur_pid++;
    assert(_cur_pid);
    //Destination
    int packet_destination = destination;
    //Potentially overwritten later
    bool record = false;
    //
    bool watch = gWatchOut && (_packets_to_watch.count(pid) > 0);


    if ((packet_destination <0) || (packet_destination >= _nodes)) {
        ostringstream err;
        err << "Incorrect packet destination " << packet_destination
            << " for stype " << _cur_pid;
        Error( err.str( ) );
    }

    if ( ( _sim_state == running ) ||
         ( ( _sim_state == draining ) && ( time < _drain_time ) ) ) {
        record = _measure_stats[cl];
    }

    if (_subnets == 1) {
        //This results in all flits having packet_type = Flit::READ_REQUEST
        subnetwork = 0;
    } else {
        assert(subnetwork < _subnets);
    }

    int extra_packet = 0;

    //Determine number of flits for packet
    if ((size % _flit_width) > 0) {
        extra_packet = 1;
    }
    size = (size / _flit_width) + extra_packet;

    //Set the flit type based on the subnetwork... assumes 4 subnetworks
    switch (subnetwork) {
        case 0:	{ packet_type = Flit::READ_REQUEST; break; }
        case 1:	{ packet_type = Flit::READ_REPLY; break; }
        case 2:	{ packet_type = Flit::WRITE_REQUEST; break; }
        case 3:	{ packet_type = Flit::WRITE_REPLY; break; }
        default:{ packet_type = Flit::ANY_TYPE; break; }
    }

    if ( watch ) {
        *gWatchOut << GetSimTime() << " | "
                   << "node" << source << " | "
                   << "Enqueuing packet " << pid
                   << " at time " << time
                   << "." << endl;
    }
    //Construct all flits required for packet
    for ( int i = 0; i < size; ++i ) {
        Flit * f  = Flit::New();
        f->id     = _cur_id++;
        assert(_cur_id);
        f->pid    = pid;
        f->watch  = watch | (gWatchOut && (_flits_to_watch.count(f->id) > 0));
        f->subnetwork = subnetwork;
        f->src    = source;
        f->ctime  = time;
        f->record = record;
        f->cl     = cl;

        //Add flit to tracking map
        _total_in_flight_flits[f->cl].insert(make_pair(f->id, f));
        if(record) {
            _measured_in_flight_flits[f->cl].insert(make_pair(f->id, f));
        }

        if(gTrace){
            cout<<"New Flit "<<f->src<<endl;
        }
        f->type = packet_type;

        if ( i == 0 ) { // Head flit
            f->head = true;
            //packets are only generated to nodes smaller or equal to limit
            f->dest = packet_destination;
        } else {
            f->head = false;
            f->dest = -1;
        }

        switch( _pri_type ) {
            case class_based:
                f->pri = _class_priority[cl];
                assert(f->pri >= 0);
                break;
            case age_based:
                f->pri = numeric_limits<int>::max() - time;
                assert(f->pri >= 0);
                break;
            case sequence_based:
                f->pri = numeric_limits<int>::max() - _packet_seq_no[source];
                assert(f->pri >= 0);
                break;
            default:
                f->pri = 0;
                break;
        }

        if ( i == ( size - 1 ) ) { // Tail flit
            f->tail = true;
        } else {
            f->tail = false;
        }

        //This flags _Step to find the first available VC
        f->vc  = -1;

        if ( f->watch ) {
            *gWatchOut << GetSimTime() << " | "
                       << "node" << source << " | "
                       << "Enqueuing flit " << f->id
                       << " (packet " << f->pid
                       << ") at time " << time
                       << "." << endl;
        }

        // Record FeS2 data for when we eject
        f->data = (void*)payload;
        _partial_packets[source][cl].push_back(f);
    }
}

void TrafficGen::_Inject() {
    RequestPacket *p;
    ReplyPacket *rp;
    for ( int input = 0; input < _nodes; ++input ) {
        for (int c = 0; c < _classes; ++c) {
            for (int subnet = 0; subnet < _subnets; ++subnet) {
                while((p = _interface->DequeueRequestPacket(input, subnet, c)) != NULL) {
                    if (p) { //generate a packet
                        if (_ideal_interconnect == 1) {
                            //Don't send through booksim, immediately eject
                            rp = new ReplyPacket;

                            rp->dest = p->dest;
                            rp->id = p->id;
                            rp->source = p->source;
                            rp->network = p->network;
                            rp->cl = p->cl;
                            _interface->EnqueueReplyPacket(rp);
                        } else {
                            //Divide packet into flits and send into booksim
                            std::cout << "src: " << p->source << "\tdst: " << p->dest << "\tbyte: " << p->size << std::endl;
                            _GeneratePacket(input, p->size, c, _time, p->id, p->network, p->dest);
                        }
                    }
                    if (p) {
                        free(p);
                        p = NULL;
                    }
                }
            }
        }
    }
}

void TrafficGen::_Step() {
    //Check for network deadlock
    bool flits_in_flight = false;

    for(int c = 0; c < _classes; ++c) {
        flits_in_flight |= !_total_in_flight_flits[c].empty();
    }

    if(flits_in_flight && (_deadlock_timer++ >= _deadlock_warn_timeout)){
        _deadlock_timer = 0;
        cout << "WARNING: Possible network deadlock.\n";
    }

    //Map of flits belonging to each subnetwork
    vector<map<int, Flit *> > flits(_subnets);

    // Step 1: Save flits to be ejected into flits vector (above)
    // Step 2: Process credits to potentially free up VCs
    for ( int subnet = 0; subnet < _subnets; ++subnet ) {
        for ( int n = 0; n < _nodes; ++n ) {
            // Potentially eject a flit from the network node n
            Flit * const f = _net[subnet]->ReadFlit(n);

            if (f) {

                if(f->watch) {
                    *gWatchOut << GetSimTime() << " | "
                               << "node" << n << " | "
                               << "Ejecting flit " << f->id
                               << " (packet " << f->pid << ")"
                               << " from VC " << f->vc
                               << "." << endl;
                }

                // Flit isn't null, save it
                flits[subnet].insert(make_pair(n, f));
                if((_sim_state == warming_up) || (_sim_state == running)) {
                    ++_accepted_flits[f->cl][n];

                    if(f->tail) {
                        ++_accepted_packets[f->cl][n];
                    }
                }

            }

            // Potentially eject a credit from network node n
            Credit * const c = _net[subnet]->ReadCredit(n);
            if ( c ) {
#ifdef TRACK_FLOWS
                for(set<int>::const_iterator iter = c->vc.begin(); iter != c->vc.end(); ++iter) {
					int const vc = *iter;
					assert(!_outstanding_classes[n][subnet][vc].empty());
					int cl = _outstanding_classes[n][subnet][vc].front();
					_outstanding_classes[n][subnet][vc].pop();
					assert(_outstanding_credits[cl][subnet][n] > 0);
					--_outstanding_credits[cl][subnet][n];
				}
#endif

                //Process the credit, thereby freeing the VC (potentially)
                _buf_states[n][subnet]->ProcessCredit(c);
                c->Free();
            }
        }

        // For all timed modules (like routers and channels), read incoming
        // inputs. The inputs come from injections from the last cycle (below),
        // however they are only stored for processing later (see Evaluate())
        _net[subnet]->ReadInputs( );
    }

    //FeS2's interface will return 1 on a quit request
    if (_sim_state != done && _interface->Step() != 0) {
        _sim_state = done;
    }

    //This will be true if the network is draining its packets
    if ( !_empty_network ) {
        _Inject();
    }

    // This for loop injects flits into their corresponding VC
    for(int subnet = 0; subnet < _subnets; ++subnet) {
        for(int n = 0; n < _nodes; ++n) {
            Flit * f = NULL;

            //The BufferState includes all VCs for the given node
            BufferState * const dest_buf = _buf_states[n][subnet];

            int const last_class = _last_class[n][subnet];
            int class_limit = _classes;

            if(_hold_switch_for_packet) {
                list<Flit *> const & pp = _partial_packets[n][last_class];

                if(!pp.empty() && !pp.front()->head &&
                   !dest_buf->IsFullFor(pp.front()->vc)) {
                    f = pp.front();

                    assert(f->vc == _last_vc[n][subnet][last_class]);

                    // if we're holding the connection, we don't need to check
                    //that class again in the for loop
                    --class_limit;
                }
            }


            for(int i = 1; i <= class_limit; ++i) {
                int const c = (last_class + i) % _classes;

                list<Flit *> const & pp = _partial_packets[n][c];

                if(pp.empty()) {
                    continue;
                }

                //Grab the flit we're meant to inject
                Flit * const cf = pp.front();
                assert(cf);
                assert(cf->cl == c);

                if(cf->subnetwork != subnet) {
                    continue;
                }

                // If we're holding the switch for another packet, skip
                if(f && (f->pri >= cf->pri)) {
                    continue;
                }

                // Find first available VC
                if(cf->head && cf->vc == -1) {

                    // Call the routing function to determine where to go
                    OutputSet route_set;
                    _rf(NULL, cf, -1, &route_set, true);

                    // Get the output set from the routing function and
                    // determine the VCs
                    set<OutputSet::sSetElement> const & os = route_set.GetSet();
                    assert(os.size() == 1);
                    OutputSet::sSetElement const & se = *os.begin();
                    assert(se.output_port == -1);
                    int vc_start = se.vc_start;
                    int vc_end = se.vc_end;
                    int vc_count = vc_end - vc_start + 1;

                    //If "next-hop-output queueing" is enabled...
                    if(_noq) {
                        assert(_lookahead_routing);
                        const FlitChannel * inject = _net[subnet]->GetInject(n);
                        const Router * router = inject->GetSink();
                        assert(router);
                        int in_channel = inject->GetSinkPort();

                        // NOTE: Because the lookahead is not for injection, but
                        // for the first hop, we have to temporarily set cf's VC
                        // to be non-negative in order to avoid seting of an
                        // assertion in the routing function.
                        cf->vc = vc_start;
                        _rf(router, cf, in_channel, &cf->la_route_set, false);
                        cf->vc = -1;

                        if(cf->watch) {
                            *gWatchOut << GetSimTime() << " | "
                                       << "node" << n << " | "
                                       << "Generating lookahead routing info for flit " << cf->id
                                       << " (NOQ)." << endl;
                        }

                        set<OutputSet::sSetElement> const sl = cf->la_route_set.GetSet();
                        assert(sl.size() == 1);
                        int next_output = sl.begin()->output_port;
                        vc_count /= router->NumOutputs();
                        vc_start += next_output * vc_count;
                        vc_end = vc_start + vc_count - 1;
                        assert(vc_start >= se.vc_start && vc_start <=se.vc_end);
                        assert(vc_end >= se.vc_start && vc_end <= se.vc_end);
                        assert(vc_start <= vc_end);
                    }


                    if(cf->watch) {
                        *gWatchOut << GetSimTime() << " | " << FullName() <<
                                   " | " << "Finding output VC for flit "
                                   << cf->id << ":" << endl;
                    }


                    for(int i = 1; i <= vc_count; ++i) {
                        // Last VC that had its buffer reserved by a flit
                        int const lvc = _last_vc[n][subnet][c];
                        // If the last VC does not conflict with the chosen VCs,
                        // use vc_start. Otherwise, attempt to choose the next
                        // VC.
                        int const vc = (lvc < vc_start || lvc > vc_end) ?
                                       vc_start :
                                       (vc_start + (lvc - vc_start + i) % vc_count);
                        assert((vc >= vc_start) && (vc <= vc_end));

                        //Check if the VC we chose is busy or full
                        if(!dest_buf->IsAvailableFor(vc)) {
                            if(cf->watch) {
                                *gWatchOut << GetSimTime() << " | " << FullName() << " | "
                                           << "  Output VC " << vc << " is busy." << endl;
                            }
                        } else {
                            if(dest_buf->IsFullFor(vc)) {
                                if(cf->watch) {
                                    *gWatchOut << GetSimTime() << " | " << FullName() << " | "
                                               << "  Output VC " << vc << " is full." << endl;
                                }
                            } else {
                                if(cf->watch) {
                                    *gWatchOut << GetSimTime() << " | " << FullName() << " | "
                                               << "  Selected output VC " << vc << "." << endl;
                                }

                                // Found a VC that is neither busy nor full
                                cf->vc = vc;
                                break;
                            }
                        }
                    }
                } //End find first available VC

                if(cf->vc == -1) {
                    if(cf->watch) {
                        *gWatchOut << GetSimTime() << " | " << FullName() << " | "
                                   << "No output VC found for flit " << cf->id
                                   << "." << endl;
                    }
                } else {
                    if(dest_buf->IsFullFor(cf->vc)) {
                        if(cf->watch) {
                            *gWatchOut << GetSimTime() << " | " << FullName() << " | "
                                       << "Selected output VC " << cf->vc
                                       << " is full for flit " << cf->id
                                       << "." << endl;
                        }
                    } else {
                        //The vc for the flit is neither busy nor full
                        f = cf;
                    }
                }
            } // End for each class

            if(f) {
                // We have a flit to inject
                assert(f->subnetwork == subnet);

                int const c = f->cl;

                if(f->head) {
                    if (_lookahead_routing) {
                        if(!_noq) {
                            const FlitChannel * inject = _net[subnet]->GetInject(n);
                            const Router * router = inject->GetSink();
                            assert(router);
                            int in_channel = inject->GetSinkPort();
                            _rf(router, f, in_channel, &f->la_route_set, false);
                            if(f->watch) {
                                *gWatchOut << GetSimTime() << " | "
                                           << "node" << n << " | "
                                           << "Generating lookahead routing info for flit " << f->id
                                           << "." << endl;
                            }
                        } else if(f->watch) {
                            *gWatchOut << GetSimTime() << " | "
                                       << "node" << n << " | "
                                       << "Already generated lookahead routing info for flit " << f->id
                                       << " (NOQ)." << endl;
                        }
                    } else {
                        f->la_route_set.Clear();
                    }

                    // This is the head flit, reserve the buffer
                    dest_buf->TakeBuffer(f->vc);
                    _last_vc[n][subnet][c] = f->vc;
                }

                _last_class[n][subnet] = c;
                _partial_packets[n][c].pop_front();

#ifdef TRACK_FLOWS
                ++_outstanding_credits[c][subnet][n];
				_outstanding_classes[n][subnet][f->vc].push(c);
#endif

                // Send the flit over the VC
                dest_buf->SendingFlit(f);

                if(_pri_type == network_age_based) {
                    f->pri = numeric_limits<int>::max() - _time;
                    assert(f->pri >= 0);
                }

                if(f->watch) {
                    *gWatchOut << GetSimTime() << " | "
                               << "node" << n << " | "
                               << "Injecting flit " << f->id
                               << " into subnet " << subnet
                               << " at time " << _time
                               << " with priority " << f->pri
                               << "." << endl;
                }
                f->itime = _time;

                // Pass VC "back" for subsequent flits of the same packet
                if(!_partial_packets[n][c].empty() && !f->tail) {
                    Flit * const nf = _partial_packets[n][c].front();
                    nf->vc = f->vc;
                }

                if((_sim_state == warming_up) || (_sim_state == running)) {
                    ++_sent_flits[c][n];
                    if(f->head) {
                        ++_sent_packets[c][n];
                    }
                }

#ifdef TRACK_FLOWS
                ++_injected_flits[c][n];
#endif

                // Send the flit over the channel
                _net[subnet]->WriteFlit(f, n);

            }
        }
    } //End for each subnetwork

    // Now eject the flits we found at the beginning of the function
    for(int subnet = 0; subnet < _subnets; ++subnet) {
        for(int n = 0; n < _nodes; ++n) {
            map<int, Flit *>::const_iterator iter = flits[subnet].find(n);
            if(iter != flits[subnet].end()) {
                Flit * const f = iter->second;

                // Flit has arrived at its destination
                f->atime = _time;
                if(f->watch) {
                    *gWatchOut << GetSimTime() << " | "
                               << "node" << n << " | "
                               << "Injecting credit for VC " << f->vc
                               << " into subnet " << subnet
                               << "." << endl;
                }

                // Send the credit over the channel
                Credit * const c = Credit::New();
                c->vc.insert(f->vc);
                _net[subnet]->WriteCredit(c, n);

#ifdef TRACK_FLOWS
                ++_ejected_flits[f->cl][n];
#endif

                // Retire the flit
                _RetireFlit(f, n);
            }
        }
        flits[subnet].clear();

        // This is where all timed modules are stepped by the network. For
        // channels, nothing happens. For routers, it's compicated.
        _net[subnet]->Evaluate( );
        // Channel: If a packet has traversed the channel according to its link
        // latency, then the flit is placed at the output, waiting to be
        // Receive()d
        // Router: Sends flits and credits over their corresponding channels.
        _net[subnet]->WriteOutputs( );
    }

    ++_time;
    assert(_time);
    if(gTrace){
        cout<<"TIME "<<_time<<endl;
    }
}

bool TrafficGen::_SingleSim() {
    int converged = 0;
    vector<double> prev_latency(_classes, 0.0);
    vector<double> prev_accepted(_classes, 0.0);
    int total_phases = 0;

    _sim_state = running;

    while (_sim_state != done) {
        for ( int iter = 0; iter < _sample_period; ++iter ) {
            _Step( );
        }

        cout << _sim_state << endl;
        cout << "Sample period: " << total_phases << endl;
        UpdateStats();
        DisplayStats();

        int lat_exc_class = -1;
        int lat_chg_exc_class = -1;
        int acc_chg_exc_class = -1;

        for(int c = 0; c < _classes; ++c) {
            if(_measure_stats[c] == 0) {
                continue;
            }

            double cur_latency = _plat_stats[c]->Average();

            int total_accepted_count;
            _ComputeStats( _accepted_flits[c], &total_accepted_count );
            double total_accepted_rate = (double)total_accepted_count /
                                         (double)(_time - _reset_time);
            double cur_accepted = total_accepted_rate / (double)_nodes;

            double latency_change = fabs((cur_latency - prev_latency[c]) / cur_latency);
            prev_latency[c] = cur_latency;

            double accepted_change = fabs((cur_accepted - prev_accepted[c]) /cur_accepted);
            prev_accepted[c] = cur_accepted;

            double latency = (double)_plat_stats[c]->Sum();
            double count = (double)_plat_stats[c]->NumSamples();

            map<int, Flit *>::const_iterator iter;
            for(iter = _total_in_flight_flits[c].begin();
                iter != _total_in_flight_flits[c].end();
                iter++) {
                latency += (double)(_time - iter->second->ctime);
                count++;
            }

            if((lat_exc_class < 0) &&
               (_latency_thres[c] >= 0.0) &&
               ((latency / count) > _latency_thres[c])) {
                lat_exc_class = c;
            }

            cout << "latency change    = " << latency_change << endl;
            if(lat_chg_exc_class < 0) {
                if((_sim_state == running) &&
                   (_stopping_threshold[c] >= 0.0) &&
                   (latency_change > _stopping_threshold[c])) {
                    lat_chg_exc_class = c;
                }
            }

            cout << "throughput change = " << accepted_change << endl;
            if(acc_chg_exc_class < 0) {
                if((_sim_state == running) &&
                   (_acc_stopping_threshold[c] >= 0.0) &&
                   (accepted_change > _acc_stopping_threshold[c])) {
                    acc_chg_exc_class = c;
                }
            }

        }

        // Fail safe for latency mode, throughput will ust continue
        if ( _measure_latency && ( lat_exc_class >= 0 ) ) {
            cout << "Average latency for class " << lat_exc_class <<
                 " exceeded " << _latency_thres[lat_exc_class] <<
                 " cycles. Aborting simulation." << endl;
            converged = 0;
            _sim_state = draining;
            _drain_time = _time;
            break;

        }

        if(_sim_state == running) {
            if ( ( !_measure_latency || ( lat_chg_exc_class < 0 ) ) &&
                 ( acc_chg_exc_class < 0 ) ) {
                ++converged;
            } else {
                converged = 0;
            }
        }
        ++total_phases;
    }
    _drain_time = _time;

    return ( true );
}