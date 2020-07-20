package ebpf

type Tracepoint struct {
	Name string
}

type KProbe struct {
	Name       string
	EntryFunc  string
	EntryEvent string
	ExitFunc   string
	ExitEvent  string
}

type PerfMapHandler func([]byte)
type PerfMapLostHandler func(uint64)
