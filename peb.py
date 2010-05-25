#!/usr/bin/env python3
#
# A module for testing the black-white pebbling number of a graph.
#
# Massimo Lauria, 2010
     

class DAG:
    def __init__(self,A):
        """
        The class defines a Direct Acyclic Graph,
        
        Parameters:
            A -- list the arcs
        """
        
        self.arcs=set( A )
        # Compute vertices
        V=set()
        V.update({a for (a,b) in self.arcs})
        V.update({b for (a,b) in self.arcs})
        
        self.vertices=list(V)

        # Computes pred and succ dictionaries
        pred=dict()
        succ=dict()

        for v in self.vertices:
            pred[v]=set()
            succ[v]=set()

        for (a,b) in self.arcs:
            pred[b].update({a})
            succ[a].update({b})
        
        self.sources={x for x in pred.keys() if not pred[x]}
        self.sinks  ={x for x in succ.keys() if not succ[x]}
        self.pred=pred
        self.succ=succ

    def id_to_bw(self,num):
        N=num
        B=set()
        W=set()
        v=0
        for i in range(len(self.vertices)):
            v=N%3
            if v==1: B.update({self.vertices[i]})
            elif v==2: W.update({self.vertices[i]})
            N = N // 3
        return (B,W)
    
    def bw_to_id(self,B,W):
        N=0
        for i in range(len(self.vertices)):
            if self.vertices[i] in B: N += (3**i) 
            elif self.vertices[i] in W: N += 2*(3**i)
        return N
        

    def path_builder(self,maxsize,start_confs=None,target_confs=None):
        """
           Build the graph of pebbling configurations
           For any possible configuration, check neighbours by
           removing a pebble (notice the the configuration graph is 
           undirected)
        """

        Visited=set()
        Pred   =dict()
        Targets=set()

        if start_confs==None:
            Queue  = [ self.bw_to_id(set(),set()) ]
        else:
            Queue  = list(start_confs)
            
        if target_confs==None:
            target_confs=lambda x: False
        

        while Queue:
            
            cid=Queue.pop(0)
            Visited.update({cid})
            if target_confs(cid):
                Targets.update({cid})
                continue

            B,W = self.id_to_bw(cid)
            P=B.union(W)
            nP = { v for v in self.vertices if v not in P}
            
            
            # Black Pebble removals
            BR=[ (B-{v},W) for v in B ]
            
            # White Pebble removals
            WR=[ (B,W-{v}) for v in W if self.pred[v] <= P ]
            
            if maxsize > len(P):
                # Black Pebble addition
                BA=[ (B.union({v}),W) for v in nP if self.pred[v] <= P ]
            
                # White Pebble addition
                WA=[ (B,W.union({v})) for v in nP if not self.pred[v] <= P ]
            else:
                BA = WA = []

            for nB,nW in BR+WR+BA+WA:
                
                nid = self.bw_to_id(nB,nW)
                
                if not (nid in Visited): 
                    Queue.append(nid)
                    Pred[nid]=cid
                   
        return Pred,Targets
                
    def find_a_pebbling(self,maxsize):

        def test_sinks(cid):
            B,W=self.id_to_bw(cid)
            return self.sinks <= B.union(W)
        
        def test_empty(cid):
            B,W=self.id_to_bw(cid)
            return len(B)==len(W)==0

        start=self.bw_to_id(set(),set())
        ARRIVAL , FULL = self.path_builder(maxsize,[start],test_sinks)

        if not FULL: return None

        DEPARTURE, _   = self.path_builder(maxsize,FULL,test_empty)

        Pebble = [start]
        try:
            while True:
                cid=Pebble[-1]
                Pebble.append(DEPARTURE[cid])
        except KeyError:
            pass
        
        try:
            while True:
                cid=Pebble[-1]
                Pebble.append(ARRIVAL[cid])
        except KeyError:
            pass

        Pebble.reverse()
        return [ self.id_to_bw(v) for v in Pebble ]
            

    @staticmethod
    def product(Do,Di):
        A=[]
        # Inner arcs
        for v in Do.vertices:
            for (a,b) in Di.arcs:
                A.append(((v,a),(v,b)))
        # Outer arcs
        for (a,b) in Do.arcs:
            for s in Di.sinks:
                for v in Di.vertices:
                    A.append( ((a,s),(b,v)) )
        return DAG(A)


    @staticmethod
    def piramid(h):
        A=[]
        if h>9: sep=":"
        else: sep=""
        for r in range(1,h):
            for c in range(h-r):
                A.append(
                    ("{0}{2}{1}".format(r-1,c,sep),
                     "{0}{2}{1}".format(r  ,c,sep)))
                A.append(
                    ("{0}{2}{1}".format(r-1,c+1,sep),
                     "{0}{2}{1}".format(r  ,c,sep)))
                
        return DAG(A)
                              
