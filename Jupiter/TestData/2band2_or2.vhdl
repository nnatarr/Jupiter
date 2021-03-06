--------------------- Cell myCell ----------------------
entity myCell is port(x0, x1, x2, x3: in BIT; y: out BIT);
  end myCell;

architecture myCell_BODY of myCell         is
  component 2band2_or2 port(X1, X2, X3, X4: in BIT; Z: out BIT);
    end component;

  signal inv_0, inv_1, inv_2, net_0, net_1: BIT;

begin
  ELM2: 2band2_or2 port map(inv_1, net_0, x1, net_1, y);
  ELM1: 2band2_or2 port map(inv_0, 0, 0, x2, net_1);
  ELM0: 2band2_or2 port map(inv_2, 0, 0, inv_0, net_0);
  PSEUDO_INVERT2: inverter port map(x3, inv_2);
  PSEUDO_INVERT1: inverter port map(x1, inv_1);
  PSEUDO_INVERT0: inverter port map(x0, inv_0);
end myCell_BODY;