#!/usr/bin/env python3
# Copyright 2023 ETH Zurich and University of Bologna.
# Licensed under the Apache License, Version 2.0, see LICENSE for details.
# SPDX-License-Identifier: Apache-2.0
#
# Author: Luca Colagrande <colluca@iis.ee.ethz.ch>

import numpy as np
import os
import sys

sys.path.append(os.path.join(os.path.dirname(__file__), "../../../../util/sim/"))
from data_utils import format_scalar_definition, format_array_definition, \
                       format_array_declaration, format_ifdef_wrapper, DataGen  # noqa: E402


class AxpyDataGen(DataGen):

    MIN = -1000
    MAX = +1000
    # AXI splits bursts crossing 4KB address boundaries. To minimize
    # the occurrence of these splits the data should be aligned to 4KB
    BURST_ALIGNMENT = 4096

    def golden_model(self, a, x, y):
        return a*x + y

    def emit_header(self, **kwargs):
        header = [super().emit_header()]

        n = kwargs['n']
        a = np.random.uniform(self.MIN, self.MAX, 1)
        x = np.random.uniform(self.MIN, self.MAX, n)
        y = np.random.uniform(self.MIN, self.MAX, n)
        g = self.golden_model(a, x, y)

        assert (n % 8) == 0, "n must be an integer multiple of the number of cores"

        header += [format_scalar_definition('const uint32_t', 'n', n)]
        header += [format_scalar_definition('const double', 'a', a[0])]
        header += [format_array_definition('double', 'x', x, alignment=self.BURST_ALIGNMENT,
                                           section=kwargs['section'])]
        header += [format_array_definition('double', 'y', y, alignment=self.BURST_ALIGNMENT,
                                           section=kwargs['section'])]
        header += [format_array_declaration('double', 'z', [n], alignment=self.BURST_ALIGNMENT,
                                            section=kwargs['section'])]
        result_def = format_array_definition('double', 'g', g)
        header += [format_ifdef_wrapper('BIST', result_def)]
        header = '\n\n'.join(header)

        return header


if __name__ == '__main__':
    sys.exit(AxpyDataGen().main())
