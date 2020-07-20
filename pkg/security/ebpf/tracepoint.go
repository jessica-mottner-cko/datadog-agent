package ebpf

import (
	"fmt"
)

func (m *Module) RegisterTracepoint(name string) error {
	if err := m.EnableTracepoint(name); err != nil {
		return fmt.Errorf("failed to load tracepoint %v: %s", name, err)
	}

	return nil
}
