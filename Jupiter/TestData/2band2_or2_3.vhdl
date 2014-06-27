-- VHDL automatically generated from
-- [2band2_or2 0 [2band2_or2 x0 x1 1 1] 0 [2band2_or2 x2 x3 1 1]]

--------------------- Cell myCell ----------------------
entity myCell is port(x0, x1, x2, x3: in BIT; y: out BIT);
  end myCell;

architecture myCell_BODY of myCell is
  component 2band2_or2 port(X1, X2, X3, X4: in BIT; Z: out BIT);
    end component;

  signal net_0, net_1: BIT;

begin
  ELM2: 2band2_or2 port map(0, net_0, 0, net_1, y);
  ELM1: 2band2_or2 port map(x2, x3, 1, 1, net_1);
  ELM0: 2band2_or2 port map(x0, x1, 1, 1, net_0);
end myCell_BODY;
