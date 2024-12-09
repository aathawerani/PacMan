import java.awt.*;
import smile.*;
import smile.learning.*;

// to run this directly from VStudio, type the following in the debugging configuration:
// Command: full path to java.exe (or just java.exe if it's in the PATH)
// Command Arguments: -ea -cp ..\java testApp ..\java\
// Working Directory: $(TargetDir)

class testApp
{
    public static void main(String[] args)
	{
        if (0 == args.length)
        {
            System.out.println("argument needed (path to directory with test files, ending with '/')");
            return;
        }
        
      	
		testDeMorgan();
		testEquations();
		
        path = args[0];
        
        String networkFile = getFullPath("credit.xdsl");
        String dataFile = getFullPath("credit.txt");
        String voiFile = getFullPath("voitest.xdsl");
        String diagFile = getFullPath("hepar.xdsl");
        String dbnFile = getFullPath("temporal.xdsl");
        String discretizationFile = getFullPath("retention.txt");
        String validationDataFile = getFullPath("tictactoe.txt");
        String validationNetFile = getFullPath("tictactoe.xdsl");
        
        testDbn(dbnFile);

        DataSet ds = loadDataSet(dataFile);
        testGtt(ds);        
        testBayesianSearch(ds);
        testTAN(ds);
        testNaiveBayes(ds);
        testPc(ds);
        testEm(networkFile, ds);
        testDataSet(dataFile);

        testNetwork(networkFile);
        testParentChildRelations();
        testSubmodels();
        testUserProperty();
        testDocItemInfo();
        
        testVoi(voiFile);
        
        testDiag(diagFile);
        
        testDeleteOutcome();
        
        testProbEvidence(networkFile);
		testVirtualEvidence(networkFile);
		
		testDiscretization(discretizationFile);
		testValidation(validationNetFile, validationDataFile);
		
		testStringOutput(networkFile);
    }

	private static void testStringOutput(String fnet)
	{
		System.out.println("*** testing string output");
		Network net1 = new Network();
		net1.readFile(fnet);
		String xdsl = net1.writeString();
		Network net2 = new Network();
		net2.readString(xdsl);
		System.out.println("net1 has " + net1.getNodeCount() + " nodes, net2 has " + net2.getNodeCount() + " nodes.");
	}
    
    private static void testValidation(String fnet, String fdata)
    {
		System.out.println("*** testing validation");
    
		Network net = new Network();
		net.readFile(fnet);
		DataSet ds = new DataSet();
		ds.readFile(fdata);

        DataMatch[] matching = ds.matchNetwork(net);
		Validator v = new Validator(net, ds, matching);	
		v.addClassNode("Situation");
		v.addClassNode("First_move");

		EM em = new EM();
		em.setUniformizeParameters(false);
		em.setRandomizeParameters(true);
		em.setSeed(1792);
		System.out.println("EM's seed is " + em.getSeed());
		System.out.println("running kfold...");
		v.kFold(em, 3, 997);
		
		DataSet dsOut = v.getResultDataSet();
		dsOut.writeFile("tmp-validator-out.txt");
		
		int[][] mtx = v.getConfusionMatrix("Situation");
		for (int row = 0; row < mtx.length; row ++)
		{
			for (int col = 0; col < mtx[row].length; col ++)
			{
				System.out.print(mtx[row][col] + " ");
			}
			System.out.println("acc=" + v.getAccuracy("Situation", row));
		}

		mtx = v.getConfusionMatrix("First_move");
		for (int row = 0; row < mtx.length; row ++)
		{
			for (int col = 0; col < mtx[row].length; col ++)
			{
				System.out.print(mtx[row][col] + " ");
			}
			System.out.println("acc=" + v.getAccuracy("First_move", row));
		}

		
		System.out.println("acc for First_move/Corner =" + v.getAccuracy("First_move", "Corner"));
    }
    
    private static void testDiscretization(String fname)
    {
		System.out.println("*** testing discretization");

        DataSet ds = loadDataSet(fname);
        double[] edges = ds.discretize(1, DataSet.DiscretizationAlgorithmType.Hierarchical, 4, "abc_");
        for (double e: edges)
        {
			System.out.println(e);
        }
        
        String[] names = ds.getStateNames(1);
        for (String n: names)
        {
			System.out.println(n);
        }
        
        int count = ds.getRecordCount();
        for (int i = 0; i < count; i ++)
        {
			System.out.print(ds.getInt(1, i));
			System.out.print(' ');
        }
        System.out.println();
        
        
        edges = ds.discretize(0, DataSet.DiscretizationAlgorithmType.UniformCount, 5, null);
        for (double e: edges)
        {
			System.out.println(e);
        }
    }
    
    private static void testVirtualEvidence(String fname)
    {
		System.out.println("*** testing virtual evidence");
		Network net = new Network();
		net.readFile(fname);

        net.setVirtualEvidence("Worth", new double[]{ 0.1, 0.3, 0.6 });
        net.setVirtualEvidence(net.getNode("Reliability"), new double[]{ 0.8, 0.2 });
		
		net.updateBeliefs();
    
		printDblArray("Virtual evidence for 'Worth'", net.getVirtualEvidence(net.getNode("Worth")));
		printDblArray("Virtual evidence for 'Reliability'", net.getVirtualEvidence("Reliability"));
		
		System.out.println("isVirtualEvidence for 'Worth' = " + net.isVirtualEvidence("Worth"));
		System.out.println("isVirtualEvidence for 'Reliability' = " + net.isVirtualEvidence(net.getNode("Reliability")));
		System.out.println("isVirtualEvidence for 'Age' = " + net.isVirtualEvidence("Age"));
    }

	private static void testDeMorgan()
	{
		System.out.println("*** testing DeMorgan nodes");
		Network net = new Network();
				
		net.addNode(Network.NodeType.DeMorgan, "a");
		net.addNode(Network.NodeType.DeMorgan, "b");
		net.addNode(Network.NodeType.DeMorgan, "c");
		
		net.addArc("c", "a");
		net.addArc("b", "a");
		
		net.setDeMorganPriorBelief("a", 0.11);
		net.setDeMorganPriorBelief("b", 0.22);
		net.setDeMorganPriorBelief("c", 0.33);
		
		net.setDeMorganParentType("a", 0, Network.DeMorganParentType.Cause);
		net.setDeMorganParentType("a", "b", Network.DeMorganParentType.Barrier);
		net.setDeMorganParentWeight(net.getNode("a"), "c", 0.44);
		net.setDeMorganParentWeight("a", 1, 0.55);
		
		net.updateBeliefs();
		
		System.out.println("prior belief for a: " + net.getDeMorganPriorBelief("a"));
		System.out.println("prior belief for b: " + net.getDeMorganPriorBelief("b"));
		System.out.println("prior belief for c: " + net.getDeMorganPriorBelief("c"));
		System.out.println("parent type of a->b: " + net.getDeMorganParentType("a", 0));
		System.out.println("parent type of a->c: " + net.getDeMorganParentType("a", "b"));
		System.out.println("parent weight of a->b: " + net.getDeMorganParentWeight("a", "c"));
		System.out.println("parent weight of a->c: " + net.getDeMorganParentWeight("a", 1));
				
		// DeMorgans only accept incoming arcs from other DeMorgans
		boolean thrown = false;
		try 
		{
			net.addNode(Network.NodeType.Cpt, "d");
			net.addArc("d", "a");
		}
		catch (SMILEException e)
		{
			thrown = true;
		}
		
		assert thrown;
	}

    private static void testEquations()
    {
		System.out.println("*** testing equations");
		
		Network net = new Network();
				
		net.addNode(Network.NodeType.Equation, "a");
		net.addNode(Network.NodeType.Equation, "b");
		net.addNode(Network.NodeType.Equation, "c");
		
		net.setNodeEquation("c", "c = 2 * normal(a, 1) + b");
		net.setNodeEquation("b", "b=5");
		net.setNodeEquationBounds("c", 3, 7);
		double [] bounds = net.getNodeEquationBounds("c");
		assert bounds[0] == 3;
		assert bounds[1] == 7;
		net.updateBeliefs();
		
		double [] value = net.getNodeValue("c");
		System.out.println("Equation: " + net.getNodeEquation("c"));
		System.out.println("Low bound: " + bounds[0]);
		System.out.println("High bound: " + bounds[1]);
		System.out.println("Mean=" + value[0]);
		System.out.println("StdDev=" + value[1]);
		
		for (int idx = 2; idx < value.length; idx ++)
		{
			System.out.print(value[idx]);
			System.out.print(" ");
		}
		System.out.println();
		
		Network net2 = new Network();
		net2.addNode(Network.NodeType.Table, "a");
		net2.addNode(Network.NodeType.Table, "b");
		net2.addNode(Network.NodeType.Table, "c");
		net2.addNode(Network.NodeType.List, "x");
		net2.addNode(Network.NodeType.Mau, "d");
		net2.addArc("x", "d");
		net2.setMauExpressions("d", new String[] { "2 * a + (b - c)/(1 + a)", "a-b/(1+exp(c))" });
		for (String s: net2.getMauExpressions("d")) 
		{
			System.out.println(s);
		}
	}


    private static void testDbn(String fname)
    {
		System.out.println("*** testing DBNs");
		
		Network net = new Network();
	
		net.readFile(fname);


		net.setSliceCount(20);
		System.out.println("slice count: " + net.getSliceCount());
		System.out.println("max temporal order: " + net.getMaxTemporalOrder());

		System.out.println("testing smile.network.unroll...");
		UnrollResults ui = net.unroll();
		System.out.println("node count in unrolled network:" + ui.unrolled.getNodeCount());
		System.out.println("entries in mapping array:" + ui.mapping.length);
		
		for (String id : net.getAllNodeIds())
		{
			if (net.getNodeTemporalType(id) == Network.NodeTemporalType.Plate)
			{
				System.out.println("node " + id + " is on temporal plate");
				System.out.println("its max temporal order: " + net.getMaxNodeTemporalOrder(id));
				System.out.println("all orders:");
				for (int ord : net.getTemporalOrders(id))
				{
					System.out.println(ord);
					for (TemporalInfo ti : net.getTemporalParents(id, ord))
					{
						System.out.println("temporal parent: " + ti.id + ", order: " + ti.order);
					}
					for (TemporalInfo ti : net.getUnrolledParents(id, ord))
					{
						System.out.println("unrolled parent: " + ti.id + ", order: " + ti.order);
					}
				}
				System.out.println();
				
				System.out.println("all unrolled parents");
				for (TemporalInfo ti : net.getUnrolledParents(id))
				{
					System.out.println("unrolled parent: " + ti.id + ", order: " + ti.order);
				}
				
				System.out.println("temporal children");
				for (TemporalInfo ti : net.getTemporalChildren(id))
				{
					System.out.println("temporal child: " + ti.id + ", order: " + ti.order);
				}
			}
			
			assert !net.hasTemporalEvidence(id);
			assert !net.isTemporalEvidence(id, 0);
		}
		
		net.setTemporalEvidence("Product_Reputation", 5, "True");
		net.setTemporalEvidence("Product_Reputation", 7, 1);
		assert net.getTemporalEvidence("Product_Reputation", 5) == 0;
		
		net.updateBeliefs();
		double[] val = net.getNodeValue("Product_Reputation");
		assert val.length == 2 * net.getSliceCount();
		for (double d : val)
		{
			System.out.print(d);
			System.out.print(" ");
		}
		System.out.println();
		
		
		net.clearTemporalEvidence("Product_Reputation", 5);
		assert !net.isTemporalEvidence("Product_Reputation", 5);
		
		net.addTemporalArc("Product_Reputation", "Large_Market_Share", 4);
		assert net.temporalArcExists("Product_Reputation", "Large_Market_Share", 4);
		assert net.getMaxTemporalOrder() == 4;
		assert net.getMaxNodeTemporalOrder("Large_Market_Share") == 4;
		
		double[] def = net.getNodeTemporalDefinition("Large_Market_Share", 4);
		for (double d : def)
		{
			System.out.print(d);
			System.out.print(" ");
		}
		double tmp = def[0];
		def[0] = def[1];
		def[1] = tmp;
		net.setNodeTemporalDefinition("Large_Market_Share", 4, def);
		System.out.println();
				
		net.deleteTemporalArc("Product_Reputation", "Large_Market_Share", 4);
		
		System.out.println("testing noisy node on the plate");
		System.out.println("definition - contains constrained (0/1) columns");
		def = net.getNodeTemporalDefinition("Product_Reputation", 2);
		for (double d : def)
		{
			System.out.print(d);
			System.out.print(" ");
		}
		System.out.println();


		System.out.println("parent strengths");
		int[][] ps = net.getNoisyTemporalParentStrengths("Product_Reputation", 2);
		for (int[] s : ps)
		{
			for (int i : s)
			{
				System.out.print(i);
				System.out.print(" ");
			}
			System.out.println();
		}
		
		ps[2][0] = 1;
		ps[2][1] = 0;
		net.setNoisyTemporalParentStrengths("Product_Reputation", 2, ps);
		System.out.println("changed parent strengths");
		ps = net.getNoisyTemporalParentStrengths("Product_Reputation", 2);
		for (int[] s : ps)
		{
			for (int i : s)
			{
				System.out.print(i);
				System.out.print(" ");
			}
			System.out.println();
		}

		System.out.println("*** DBN test complete, press Enter to continue");
    	System.out.println();
    }

    private static void testProbEvidence(String fname)
    {
        Network net = new Network();
        net.readFile(fname);
        
        net.setEvidence("Worth", "High");
        net.setEvidence("Reliability", "Reliable");
        
        double p = net.probEvidence();
        System.out.println("probEvidence returns " + p);
        
        String[] mapNodes = new String[2];
        mapNodes[0] = "Income";
        mapNodes[1] = "FutureIncome";
        
        System.out.println("Running annealedMap");
        AnnealedMapResults am = net.annealedMap(mapNodes, null);
        
        System.out.println("pm1e = " + am.probM1E);
        System.out.println("pe = " + am.probE);
        System.out.println("state0 = " + net.getOutcomeId(mapNodes[0], am.mapStates[0]));
        System.out.println("state1 = " + net.getOutcomeId(mapNodes[1], am.mapStates[1]));
    }

    private static void testDeleteOutcomeHelper(Network net, int handle)
    {
        System.out.println("outcome count: " + net.getOutcomeCount(handle));
        net.addOutcome(handle, "x1");
        System.out.println("outcome count: " + net.getOutcomeCount(handle));
        net.addOutcome(handle, "x2");
        System.out.println("outcome count: " + net.getOutcomeCount(handle));
        net.deleteOutcome(handle, 0);
        System.out.println("outcome count: " + net.getOutcomeCount(handle));
        net.deleteOutcome(handle, 0);
        System.out.println("outcome count: " + net.getOutcomeCount(handle));
        
        try
        {
            System.out.println("node has 2 outcomes now, trying to remove another outcome");
            net.deleteOutcome(handle, 0);
        }
        catch (SMILEException e)
        {
            System.out.println("exception, as expected");
        }
    }

    
    private static void testDeleteOutcome()
    {
        Network net = new Network();
        int handleCpt = net.addNode(Network.NodeType.Cpt);
        testDeleteOutcomeHelper(net, handleCpt);
        int handleNoisy = net.addNode(Network.NodeType.NoisyMax);
        testDeleteOutcomeHelper(net, handleNoisy);
    }
    
    private static void testDocItemInfo()
    {
        Network net = new Network();
        String nodeId = "id1";
        String outcomeId = "newOutcomeId";
        int handle = net.addNode(Network.NodeType.Cpt, nodeId);
        net.addOutcome(handle, outcomeId);
        int count = 500;
        DocItemInfo[] di = new DocItemInfo[count];
        for (int i = 0; i < count; i ++)
        {
            String suffix = new String();
            suffix = suffix + i;
            di[i] = new DocItemInfo("title" + suffix, "path" + suffix);
            //System.out.println(di[i].title + " " + di[i].path);
        }

        net.setNodeDocumentation(nodeId, di);
        net.setOutcomeDocumentation(nodeId, outcomeId, di);
        
        DocItemInfo[] ret1 = net.getNodeDocumentation(nodeId);
        DocItemInfo[] ret2 = net.getOutcomeDocumentation(nodeId, outcomeId);
        assert ret1.length == di.length;
        assert ret2.length == di.length;
        for (int i = 0; i < count; i ++)
        {
            assert di[i].title.equals(ret1[i].title);
            assert di[i].path.equals(ret1[i].path);
            assert di[i].title.equals(ret2[i].title);
            assert di[i].path.equals(ret2[i].path);
        }
    }
    
    private static void testUserProperty()
    {
        Network net = new Network();
        String nodeId = "id1";
        int handle = net.addNode(Network.NodeType.Cpt, nodeId);
        
        int count = 10;
        UserProperty[] up = new UserProperty[count];
        for (int i = 0; i < count; i ++)
        {
            String suffix = new String();
            suffix = suffix + i;
            up[i] = new UserProperty("prop" + suffix, "val" + suffix);
            //System.out.println(up[i].name + " " + up[i].value);
        }
        
        net.setUserProperties(up);
        net.setNodeUserProperties(nodeId, up);
        
        UserProperty[] ret1 = net.getUserProperties();
        UserProperty[] ret2 = net.getNodeUserProperties(nodeId);
        assert ret1.length == up.length;
        assert ret2.length == up.length;
        for (int i = 0; i < count; i ++)
        {
            assert up[i].name.equals(ret1[i].name);
            assert up[i].value.equals(ret1[i].value);
            assert up[i].name.equals(ret2[i].name);
            assert up[i].value.equals(ret2[i].value);
        }
    }    
    
    private static void testSubmodels()
    {
        Network net = new Network();
        String id1 = "id1";
        String id2 = "id2";
        String sub1 = "sub1";
        String sub2 = "sub2";
        int h1 = net.addNode(Network.NodeType.Cpt, id1);
        int h2 = net.addNode(Network.NodeType.Cpt, id2);
        
        int sMain = net.getMainSubmodel();
        int s1 = net.addSubmodel(sMain, sub1);
        int s2 = net.addSubmodel(net.getMainSubmodelId(), sub2);
        
        net.setSubmodelId(sMain, "mainId");
        net.setSubmodelName(sMain, "mainName");
        net.setSubmodelDescription(sMain, "some description of main submodel");
        
        System.out.println("Main submodel id:   " + net.getSubmodelId(sMain));
        System.out.println("Main submodel name: " + net.getSubmodelName(sMain));
        System.out.println("Main submodel desc: " + net.getSubmodelDescription(sMain));
        
        
        net.setSubmodelOfSubmodel(sub1, sub2);
        net.setSubmodelOfNode(s1, h1);
        net.setSubmodelOfNode(sub2, id2);
        
        System.out.println("sub of node1: " + net.getSubmodelId(net.getSubmodelOfNode(h1)));
        System.out.println("sub of node2: " + net.getSubmodelId(net.getSubmodelOfNode(id2)));
        System.out.println("sub of sub1:  " + net.getSubmodelId(net.getSubmodelOfSubmodel(s1)));
        System.out.println("sub of sub2:  " + net.getSubmodelId(net.getSubmodelOfSubmodel(sub2)));
        
        net.setSubmodelPosition(sub1, 100, 100, 155, 44);
        System.out.println("pos of sub1 changed to " + net.getSubmodelPosition(s1));        
        net.setSubmodelPosition(s1, new Rectangle(200, 200, 44, 155));
        System.out.println("and now it is " + net.getSubmodelPosition(sub1));        
        
        net.deleteSubmodel(sub2);
        net.deleteSubmodel(s1);
        
        System.out.println();
    }
    
    private static void testDiag(String fname)
    {
        System.out.println("Testing smile.DiagNetwork");
        Network net = new Network();
        net.readFile(fname);
        
        DiagNetwork diag = new DiagNetwork(net);
        System.out.println("diag.getPursuedFault: " + diag.getPursuedFault());

        System.out.println("calling update...");
        DiagResults res = diag.update();
        
        System.out.println("observations:");
        for (ObservationInfo oi : res.observations)
        {
            System.out.print(oi.node);
            System.out.print(" ");
            System.out.print(oi.entropy);
            System.out.print(" ");
            System.out.print(oi.cost);
            System.out.print(" ");
            System.out.print(oi.infoGain);
            System.out.println();
        }

        System.out.println("faults:");
        for (FaultInfo fi : res.faults)
        {
            System.out.print(fi.node);
            System.out.print(" ");
            System.out.print(fi.outcome);
            System.out.print(" ");
            System.out.print(fi.probability);
            System.out.print(" ");
            System.out.print(fi.isPursued);
            System.out.println();
        }
        
        System.out.println("faults using DiagNetwork.getFault");
        int count = diag.getFaultCount();
        for (int i = 0; i < count; i ++)
        {
            FaultInfo fi = diag.getFault(i);
            System.out.print(i);
            System.out.print(" ");
            System.out.print(fi.index);
            System.out.print(" ");
            System.out.print(fi.node);
            System.out.print(" ");
            System.out.print(fi.outcome);
            System.out.print(" ");
            System.out.print(fi.probability);
            System.out.print(" ");
            System.out.print(fi.isPursued);
            System.out.println();

        }
        
        System.out.println();
    }
    
    private static void testVoi(String fname)
    {
        System.out.println("Testing smile.ValueOfInfo");
        Network net = new Network();
        net.readFile(fname);
        ValueOfInfo voi = new ValueOfInfo(net);
        voi.addNode("Forecast");
        voi.setDecision("Invest");
        voi.update();
        printDblArray("voi.getValues", voi.getValues());
        printStrArray("voi.getAllNodeIds", voi.getAllNodeIds());
        System.out.println("voi.getPointOfViewId: " + voi.getPointOfViewId());
        System.out.println("voi.getDecisionId: " + voi.getDecisionId());
        System.out.println();
    }

    private static void testNetwork(String fname)
    {
        System.out.println("Testing smile.Network");
        Network net = new Network();
        System.out.println("Reading file: " + fname);
        net.readFile(fname);
        System.out.println("Node count: " + net.getNodeCount());
        
        System.out.println("Color of income node (should be red):   " + net.getNodeBgColor("Income"));
        System.out.println("Color of debit node (should be green):  " + net.getNodeBgColor("Debit"));
        int assets = net.getNode("Assets");
        System.out.println("Color of assets node (should be blue):  " + net.getNodeBgColor(assets));
        System.out.println("Color of assets text (should be white): " + net.getNodeTextColor(assets));
        System.out.println("Color of assets border (should be blk): " + net.getNodeBorderColor(assets));
        System.out.println("Changing assets to red bg, green text and blue border");
        net.setNodeBgColor(assets, Color.RED);
        net.setNodeTextColor(assets, Color.GREEN);
        net.setNodeBorderColor(assets, Color.BLUE);
        System.out.println("New colors: ");
        System.out.println("Color of assets node (should be red):    " + net.getNodeBgColor(assets));
        System.out.println("Color of assets text (should be green):  " + net.getNodeTextColor(assets));
        System.out.println("Color of assets border (should be blue): " + net.getNodeBorderColor(assets));
        
        System.out.println("Border width of income node (should be 1): " + net.getNodeBorderWidth("Income"));
        System.out.println("Border width of assets node (should be 2): " + net.getNodeBorderWidth(assets));
        net.setNodeBorderWidth(assets, 3);
        System.out.println("Border width of assets node (should be 3 now): " + net.getNodeBorderWidth(assets));
        
        net.setNodePosition(assets, 0, 0, 100, 100);
        System.out.println("moved assets to top-left corner, pos is: " + net.getNodePosition(assets));
        net.setNodePosition(assets, new Rectangle(100, 100, 200, 200));
        System.out.println("moved it to 100/100 and increased size, pos is: " + net.getNodePosition(assets));
        
        System.out.println("changing two nodes to noisy " + Network.NodeType.NoisyMax);
        net.setNodeType(assets, Network.NodeType.NoisyMax);
        net.setNodeType("Income", Network.NodeType.NoisyMax);
        System.out.println("now assets is of type: " + net.getNodeType(assets));
        System.out.println("now income is of type: " + net.getNodeType("Income"));
        System.out.println("but debit is unchanged: " + net.getNodeType("Debit"));
        
        System.out.println();
    }

    private static void testParentChildRelations()
    {
        System.out.println("More tests on smile.Network");
        
        Network net = new Network();
        
        System.out.println("New network created, checking header values (id/name/description):");
        System.out.println(net.getId());
        System.out.println(net.getName());
        System.out.println(net.getDescription());
        System.out.println("done.");
        
        String id1 = "id1";
        String id2 = "id2";
        String id3 = "id3";
        
        int h1 = net.addNode(Network.NodeType.Cpt, id1);
        int h2 = net.addNode(Network.NodeType.Cpt, id2);
        int h3 = net.addNode(Network.NodeType.Cpt, id3);
        
        net.addArc(h1, h2);
        net.addArc(id1, id3);

        printIntArray("net.getChildren", net.getChildren(h1));
        printStrArray("net.getChildIds", net.getChildIds(id1));
        printIntArray("net.getParents", net.getParents(h2));
        printStrArray("net.getParentIds", net.getParentIds(id3));
        
        net.addCostArc(h2, h1);
        net.addCostArc(id3, id1);

        printIntArray("net.getCostParents", net.getCostParents(h1));
        printStrArray("net.getCostParentIds", net.getCostParentIds(id1));
        printIntArray("net.getCostChildren", net.getCostChildren(h2));
        printStrArray("net.getCostChildIds", net.getCostChildIds(id3));

        net.setNodeCost(id1, new double[]{1,2,3,4});
        printDblArray("net.getNodeCost", net.getNodeCost(h1));

        
        net.setRanked(h1, true);
        net.setRanked(id2, true);
        net.setMandatory(h1, true);
        net.setMandatory(id2, true);
                
        System.out.println("isRanked: " + net.isRanked(id1));
        System.out.println("isRanked: " + net.isRanked(h2));
        System.out.println("isMandatory: " + net.isMandatory(id1));
        System.out.println("isMandatory: " + net.isMandatory(h2));
        
        net.deleteArc(id1, id2);
        
        net.setNodeType(id3, Network.NodeType.NoisyMax);
        net.setNoisyParentStrengths(id3, id1, new int[] { 0, 1 });
        
        net.deleteArc(h1, h3);
        net.deleteCostArc(id2, id1);
        net.deleteCostArc(h3, h1);
        
        System.out.println();
    }
    
    private static void showNetInfo(Network net)
    {
        System.out.println("Node count: " + net.getNodeCount());
        int count = 0;
        for (int h = net.getFirstNode(); h >= 0; h = net.getNextNode(h))
        {
            if (net.getParents(h).length == 0) count ++;
        }
        System.out.println("Toplevel nodes: " + count);
    }

    private static void testTAN(DataSet ds)
    {
        System.out.println("Testing smile.learning.TAN");

        TAN tan = new TAN();
        String classVar = ds.getVariableId(0);
        tan.setClassVariableId(classVar);
        System.out.println("Class variable id is " + tan.getClassVariableId());
        
        tan.setMaxSearchTime(1000);
        tan.setRandSeed(997);
		System.out.println("rndSeed=" + tan.getRandSeed());
        System.out.println("maxTime=" + tan.getMaxSearchTime());
        
        Network net = tan.learn(ds);
        showNetInfo(net);
        System.out.println("Class variable children count: " + net.getChildren(classVar).length);
        
        net.writeFile("tmp-tan.xdsl");
    }
    
    private static void testNaiveBayes(DataSet ds)
    {
        System.out.println("Testing smile.learning.NaiveBayes");

        NaiveBayes nb = new NaiveBayes();
        String classVar = ds.getVariableId(0);
        nb.setClassVariableId(classVar);
        System.out.println("Class variable id is " + nb.getClassVariableId());
        
        nb.setFeatureSelection(false);
        nb.setPriorsMethod(NaiveBayes.PriorsType.BDeu);
        nb.setNetWeight(1.2345);
        
        System.out.println("featSel=" + nb.getFeatureSelection());
        System.out.println("priMeth=" + nb.getPriorsMethod());
        System.out.println("netWght=" + nb.getNetWeight());
        
        Network net = nb.learn(ds);
        showNetInfo(net);
        System.out.println("Class variable children count: " + net.getChildren(classVar).length);
        
        net.writeFile("tmp-nb.xdsl");
    }
    
    private static void testPc(DataSet ds)
    {
        System.out.println("Testing smile.learning.PC");

        PC pc = new PC();
        
        pc.setSignificance(0.34567);
        pc.setMaxAdjacency(3);
        
        System.out.println("Signif=" + pc.getSignificance());
        System.out.println("maxAdj=" + pc.getMaxAdjacency());

        BkKnowledge bkk = pc.getBkKnowledge();
        assert bkk.getVariableCount() == 0;
        
        Pattern pat = pc.learn(ds);
        System.out.println("pattern size: " + pat.getSize());
		System.out.println("pattern is DAG? " + pat.isDAG());
		System.out.println("pattern has cycles? " + pat.hasCycle());
	
		int count = pat.getSize();
		for (int i = 0; i < count; i ++)
		{
			for (int j = 0; j < count; j ++)
			{
				switch (pat.getEdge(i, j))
				{
				case Pattern.EdgeType.Directed:
					if (i > j) pat.setEdge(i, j, Pattern.EdgeType.None);
					break;
				case Pattern.EdgeType.Undirected:
					pat.setEdge(i, j, i > j ? Pattern.EdgeType.None : Pattern.EdgeType.Directed);
					break;
				}
			}
		}
	
		Network net = pat.makeNetwork(ds);
		net.writeFile("tmp-pc.xdsl");
    }

    private static void testBayesianSearch(DataSet ds)
    {
        System.out.println("Testing smile.learning.BayesianSearch");
        BayesianSearch bs = new BayesianSearch();
        
        bs.setMaxParents(5);
        bs.setIterationCount(20);
        bs.setRandSeed(997);
        bs.setPriorSampleSize(50);
        bs.setLinkProbability(0.1);
        bs.setPriorLinkProbability(0.001);
        bs.setMaxSearchTime(7);
        
        
        System.out.println("maxParn=" + bs.getMaxParents());
        System.out.println("iterCnt=" + bs.getIterationCount());
        System.out.println("rndSeed=" + bs.getRandSeed());
        System.out.println("smpSize=" + bs.getPriorSampleSize());
        System.out.println("maxTime=" + bs.getMaxSearchTime());
        System.out.println("lnkProb=" + bs.getLinkProbability());
        System.out.println("priProb=" + bs.getPriorLinkProbability());
                
        BkKnowledge bkk = new BkKnowledge();
        bkk.matchData(ds);
        bkk.addForcedArc(0, 1);
        bkk.addForcedArc(0, 2);
        bkk.addForcedArc(0, 3);
        bkk.addForbiddenArc(3, 4);
        bkk.addForbiddenArc(3, 5);
        bkk.addForbiddenArc(3, 6);
		bkk.setTier(7, 1);
		bkk.setTier(8, 2);
		bkk.setTier(9, 2);
                
        bs.setBkKnowledge(bkk);

        bkk = bs.getBkKnowledge();
        assert bkk.getForcedChildren(0).length == 3;
        assert bkk.getForbiddenChildren(3).length == 3;
        assert bkk.getTier(8) == 2;
        
        bkk.writeFile("tmp-bkk.gkno");

		BkKnowledge bkkRoundtrip = new BkKnowledge();
		bkkRoundtrip.readFile("tmp-bkk.gkno");
        assert bkkRoundtrip.getForcedChildren(0).length == 3;
        assert bkkRoundtrip.getForbiddenChildren(3).length == 3;
        assert bkkRoundtrip.getTier(8) == 2;
        
        Network net = bs.learn(ds);
        showNetInfo(net);
        
        System.out.println("Last score=" + bs.getLastScore());
        
        net.writeFile("tmp-bs.xdsl");
    }


    private static void testGtt(DataSet ds)
    {
        System.out.println("Testing smile.learning.GreedyThickThinning");
        GreedyThickThinning gtt = new GreedyThickThinning();
        
        gtt.setNetWeight(2.3456);
        gtt.setPriorsMethod(GreedyThickThinning.PriorsType.BDeu);
        gtt.setMaxParents(5);
        
        System.out.println("maxParn=" + gtt.getMaxParents());
        System.out.println("priMeth=" + gtt.getPriorsMethod());
        System.out.println("netWght=" + gtt.getNetWeight());
        
        BkKnowledge bkk = new BkKnowledge();
        bkk.matchData(ds);
        bkk.addForcedArc(0, 1);
        bkk.addForcedArc(0, 2);
        bkk.addForcedArc(0, 3);
        bkk.addForbiddenArc(3, 4);
        bkk.addForbiddenArc(3, 5);
        bkk.addForbiddenArc(3, 6);
		bkk.setTier(7, 1);
		bkk.setTier(8, 2);
		bkk.setTier(9, 2);
                
        gtt.setBkKnowledge(bkk);

        bkk = gtt.getBkKnowledge();
        assert bkk.getForcedChildren(0).length == 3;
        assert bkk.getForbiddenChildren(3).length == 3;
        assert bkk.getTier(8) == 2;
        
        bkk.writeFile("tmp-bkk.gkno");

		BkKnowledge bkkRoundtrip = new BkKnowledge();
		bkkRoundtrip.readFile("tmp-bkk.gkno");
        assert bkkRoundtrip.getForcedChildren(0).length == 3;
        assert bkkRoundtrip.getForbiddenChildren(3).length == 3;
        assert bkkRoundtrip.getTier(8) == 2;
        
        Network net = gtt.learn(ds);
        showNetInfo(net);
        
        net.writeFile("tmp-gtt.xdsl");
    }

    private static void testEm(String networkFile, DataSet ds)
    {
        System.out.println("Testing smile.learning.EM");
        Network net = new Network();
        net.readFile(networkFile);
        System.out.println("Network loaded from " + networkFile);
        
		System.out.println("calling matchNetwork...");
        DataMatch[] matching = ds.matchNetwork(net);
        for (DataMatch m : matching)
        {
			System.out.println("Matched column " + m.column + " with node " + net.getNodeId(m.node));
        }
        
        String[] fixedNodes = { "RatioDebInc", "Worth" };
        
        System.out.print("Definition of fixed node before EM:");
        for (double d : net.getNodeDefinition("Worth")) System.out.print(" " + d);
        System.out.println();
        
        EM em = new EM();
        em.setEqSampleSize(10);
		em.setRandomizeParameters(false);
        em.setRelevance(false);
        System.out.println("eqSampl=" + em.getEqSampleSize());
        System.out.println("randPar=" + em.getRandomizeParameters());
		System.out.println("relevance=" + em.getRelevance());
        em.learn(ds, net, matching, fixedNodes);
        System.out.println("EM is complete");
        System.out.println();

        System.out.print("Definition of fixed node after EM:");
        for (double d : net.getNodeDefinition("Worth")) System.out.print(" " + d);
        System.out.println();
		System.out.println();
		
		System.out.println("EM log likelihood=" + em.getLastScore());
        
        net.writeFile("tmp-em.xdsl");
    }
    
    
    private static DataSet loadDataSet(String fname)
    {
        DataSet ds = new DataSet();
        System.out.println("Reading data file: " + fname);
        ds.readFile(fname);
        return ds;
    }
    
     
    private static void testDataSet(String fname)
    {
        System.out.println("Testing smile.learning.DataSet");
        DataSet ds = loadDataSet(fname);
        System.out.println("Invoking GC");
        System.gc();
        System.out.println("Var count: " + ds.getVariableCount());
        System.gc();
        System.out.println("Rec count: " + ds.getRecordCount());
        
        System.out.println("Creating new dataset, adding int and float variable");
        DataSet ds2 = new DataSet();
        ds2.addIntVariable("x1");
        ds2.addFloatVariable("x2");

        String[] stateNames = new String[10];
        for (int i = 0; i < 10; i ++)
        {
            stateNames[i] = new String("State" + i);
        }
        System.out.println("Setting state names for int variable");
        ds2.setStateNames(0, stateNames);        

        System.out.println("Reading state names back");
        printStrArray("ds2.getStateNames", ds2.getStateNames(0));
        
        System.out.println("Adding records");
        final int REC_COUNT = 1000;
        for (int i = 0; i < REC_COUNT; i ++)
        {
            ds2.addEmptyRecord();
            ds2.setInt(0, i, 10 * i + 5);
            ds2.setFloat(1, i, 1234.567f * i);
        }
        
        System.out.println("Var count: " + ds2.getVariableCount());
        System.out.println("Rec count: " + ds2.getRecordCount());
        
        for (int i = 0; i < ds2.getVariableCount(); i ++)
        {
			System.out.println("Is variable #" + i + " discrete? " + ds2.isDiscrete(i));
			String varId = ds2.getVariableId(i);
			System.out.println("Is " + varId + " discrete? " + ds2.isDiscrete(varId));
        }
        
        System.out.println("Reading data back");
        int isum = 0;
        float fsum = 0;
        for (int i = 0; i < ds2.getRecordCount(); i ++)
        {
            isum += ds2.getInt(0, i);
            fsum += ds2.getFloat(1, i);
        }
        
        int expectedIsum = 5 * REC_COUNT + 10 * REC_COUNT * (REC_COUNT - 1) / 2;
        float expectedFsum = 1234.567f * REC_COUNT * (REC_COUNT - 1) / 2;
        
        assert expectedIsum == isum;
        assert expectedFsum == fsum;
        System.out.println("isum=" + isum + ", expected: " + expectedIsum);
        System.out.println("fsum=" + fsum + ", expected: " + expectedFsum);
        
        ds2.setMissing(0, 0);
        ds2.setMissing(1, 0);
        
        assert ds2.isMissing(0, 0);
        assert ds2.isMissing(1, 0);
        
        assert !ds2.isMissing(0, 1);
        assert !ds2.isMissing(1, 1);
        
        System.out.println("Writing dataset...");
        ds.writeFile("tmp-ds-roundtrip.txt");
        System.out.println("Dataset file saved.");
     
        System.out.println();
    }
    
    private static String path;
    
    private static String getFullPath(String coreName)
    {
        return path + coreName;
    }
    
    private static void printIntArray(String caption, int[] a)
    {
        System.out.print(caption + ": ");
        for (int x : a) 
        {
            System.out.print(x);
            System.out.print(" ");
        }
        System.out.println();
    }

    private static void printStrArray(String caption, String[] a)
    {
        System.out.print(caption + ": ");
        for (String x : a) 
        {
            System.out.print(x);
            System.out.print(" ");
        }
        System.out.println();
    }

    private static void printDblArray(String caption, double[] a)
    {
        System.out.print(caption + ": ");
        for (double x : a) 
        {
            System.out.print(x);
            System.out.print(" ");
        }
        System.out.println();
    }
}
